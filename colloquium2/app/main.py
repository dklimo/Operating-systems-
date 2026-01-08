from fastapi import FastAPI, HTTPException, Depends, status, Request
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
from typing import List
import structlog
from prometheus_client import Counter, Histogram, generate_latest
from prometheus_client import CONTENT_TYPE_LATEST
from datetime import datetime
from app import schemas, crud, models
from app.database import get_db, init_db
from app.config import settings


logger = structlog.get_logger()


REQUESTS = Counter('http_requests_total', 'Total HTTP Requests', ['method', 'endpoint', 'status'])
REQUEST_DURATION = Histogram('http_request_duration_seconds', 'HTTP request duration in seconds',
                             ['method', 'endpoint'])

app = FastAPI(
    title=settings.app_name,
    description="REST API для управления задачами (To-Do List)",
    version="1.0.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.middleware("http")
async def add_logging_and_metrics(request: Request, call_next):
    method = request.method
    endpoint = request.url.path

    with REQUEST_DURATION.labels(method=method, endpoint=endpoint).time():
        response = await call_next(request)

    REQUESTS.labels(method=method, endpoint=endpoint, status=response.status_code).inc()

    logger.info(
        "request",
        method=method,
        endpoint=endpoint,
        status_code=response.status_code,
        client_ip=request.client.host if request.client else None
    )

    return response


@app.get("/metrics")
async def metrics():
    return JSONResponse(
        content=generate_latest().decode('utf-8'),
        media_type=CONTENT_TYPE_LATEST
    )


@app.on_event("startup")
async def startup_event():
    """Инициализация при запуске"""
    init_db()
    logger.info("Application started", app_name=settings.app_name)


@app.get("/")
async def root():
    """Корневой эндпоинт"""
    return {
        "message": "Добро пожаловать в To-Do List API",
        "documentation": "/docs",
        "version": "1.0.0"
    }


@app.get("/health")
async def health_check():
    """Проверка здоровья API"""
    return {"status": "healthy", "timestamp": datetime.utcnow().isoformat()}


@app.get("/tasks", response_model=List[schemas.TaskResponse])
def read_tasks(
        skip: int = 0,
        limit: int = 100,
        search: str = None,
        db: Session = Depends(get_db)
):
    """
    Получить список задач

    - **skip**: Пропустить первые N записей
    - **limit**: Максимальное количество записей
    - **search**: Поиск по названию или описанию
    """
    if search:
        tasks = crud.TaskCRUD.search_tasks(db, search)
    else:
        tasks = crud.TaskCRUD.get_tasks(db, skip=skip, limit=limit)
    return tasks


@app.post("/tasks", response_model=schemas.TaskResponse, status_code=status.HTTP_201_CREATED)
def create_task(
        task: schemas.TaskCreate,
        db: Session = Depends(get_db)
):
    """
    Создать новую задачу

    - **title**: Название задачи (обязательно)
    - **description**: Описание задачи (опционально)
    - **status**: Статус задачи (todo/in_progress/done)
    """
    try:
        db_task = crud.TaskCRUD.create_task(db, task)
        logger.info("task_created", task_id=db_task.id, title=db_task.title)
        return db_task
    except Exception as e:
        logger.error("task_creation_failed", error=str(e))
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Не удалось создать задачу"
        )


@app.get("/tasks/{task_id}", response_model=schemas.TaskResponse)
def read_task(
        task_id: int,
        db: Session = Depends(get_db)
):
    """
    Получить задачу по ID

    - **task_id**: ID задачи
    """
    db_task = crud.TaskCRUD.get_task(db, task_id)
    if db_task is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Задача не найдена"
        )
    return db_task


@app.put("/tasks/{task_id}", response_model=schemas.TaskResponse)
def update_task_full(
        task_id: int,
        task_update: schemas.TaskUpdate,
        db: Session = Depends(get_db)
):
    """
    Полностью обновить задачу (PUT)

    - **task_id**: ID задачи
    - Все поля задачи
    """
    db_task = crud.TaskCRUD.update_task(db, task_id, task_update)
    if db_task is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Задача не найдена"
        )
    logger.info("task_updated", task_id=task_id)
    return db_task


@app.patch("/tasks/{task_id}", response_model=schemas.TaskResponse)
def update_task_partial(
        task_id: int,
        task_update: schemas.TaskUpdate,
        db: Session = Depends(get_db)
):
    """
    Частично обновить задачу (PATCH)

    - **task_id**: ID задачи
    - Любое из полей задачи
    """
    db_task = crud.TaskCRUD.update_task(db, task_id, task_update)
    if db_task is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Задача не найдена"
        )
    logger.info("task_partially_updated", task_id=task_id)
    return db_task


@app.delete("/tasks/{task_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_task(
        task_id: int,
        db: Session = Depends(get_db)
):
    """
    Удалить задачу

    - **task_id**: ID задачи
    """
    success = crud.TaskCRUD.delete_task(db, task_id)
    if not success:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Задача не найдена"
        )
    logger.info("task_deleted", task_id=task_id)
    return None


@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    logger.warning(
        "http_exception",
        status_code=exc.status_code,
        detail=exc.detail,
        path=request.url.path
    )
    return JSONResponse(
        status_code=exc.status_code,
        content={"detail": exc.detail},
    )


@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(
        "unhandled_exception",
        error=str(exc),
        path=request.url.path,
        exc_info=True
    )
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content={"detail": "Внутренняя ошибка сервера"},
    )