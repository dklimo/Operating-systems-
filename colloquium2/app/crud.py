from sqlalchemy.orm import Session
from app import models, schemas
from typing import List, Optional
from app.database import redis_client
import json
from datetime import datetime, timedelta


class TaskCRUD:
    @staticmethod
    def get_task(db: Session, task_id: int):
        """Получить задачу по ID"""
        cache_key = f"task:{task_id}"
        cached_task = redis_client.get(cache_key)

        if cached_task:
            return json.loads(cached_task)

        task = db.query(models.Task).filter(models.Task.id == task_id).first()

        if task:
            task_data = schemas.TaskResponse.from_orm(task).dict()
            redis_client.setex(
                cache_key,
                timedelta(minutes=5),
                json.dumps(task_data, default=str)
            )

        return task

    @staticmethod
    def get_tasks(db: Session, skip: int = 0, limit: int = 100):
        return db.query(models.Task).offset(skip).limit(limit).all()

    @staticmethod
    def create_task(db: Session, task: schemas.TaskCreate):
        db_task = models.Task(
            title=task.title,
            description=task.description,
            status=task.status
        )
        db.add(db_task)
        db.commit()
        db.refresh(db_task)
        return db_task

    @staticmethod
    def update_task(db: Session, task_id: int, task_update: schemas.TaskUpdate):
        db_task = db.query(models.Task).filter(models.Task.id == task_id).first()
        if not db_task:
            return None

        update_data = task_update.dict(exclude_unset=True)
        for field, value in update_data.items():
            setattr(db_task, field, value)

        db_task.updated_at = datetime.utcnow()
        db.commit()
        db.refresh(db_task)

        redis_client.delete(f"task:{task_id}")

        return db_task

    @staticmethod
    def delete_task(db: Session, task_id: int):
        db_task = db.query(models.Task).filter(models.Task.id == task_id).first()
        if not db_task:
            return False

        db.delete(db_task)
        db.commit()
        redis_client.delete(f"task:{task_id}")

        return True

    @staticmethod
    def search_tasks(db: Session, query: str):
        return db.query(models.Task).filter(
            models.Task.title.contains(query) |
            models.Task.description.contains(query)
        ).all()