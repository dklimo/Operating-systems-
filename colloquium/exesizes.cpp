#include "exesizes.h"

vector<int> fibo(int n) {
	vector <int> result;
	if (n <= 0) return result;
	if (n >= 1)
		result.push_back(0);
	if (n >= 2)
		result.push_back(1);
	for (int i = 2; i < n;i++) {
		result.push_back(result[i - 1] + result[i - 2]);
	}
	return result;
}

bool palindrome(int n) {
	if (n < 0) return false;
	int org = n;
	long long rev = 0;
	while (n > 0) {
		rev = rev * 10 + n % 10;
		n /= 10;
	}
	return org == rev;
}

ListN* Reverse(ListN* head) {
	ListN* prev = nullptr;
	ListN* curr = head;
	ListN* next = nullptr;

	while (curr != nullptr) {
		next = curr->next;
		curr->next = prev;

		prev = curr;
		curr = next;
	}
	return prev;
}
void PrintResult(int n1, int n2, ListN* head) {
	vector<int> f = fibo(n1);
	cout << "Fibonacci numbers: ";
	for (int num : f) {
		cout << num << ' ';
	}
	cout << endl;

	cout << "Number " << n2 << " is palindrome: ";
	if (palindrome(n2)) {
		cout << "YES";
	}
	else {
		cout << "NO";
	}
	cout << endl;

	head = Reverse(head);
	cout << "Reversed linked list: ";
	ListN* curr = head;
	while (curr != nullptr) {
		cout << curr->value << ' ';
		curr = curr->next;
	}
	cout << endl;
}
ListN* inputList() {
	int n;
	cout << "Enter number of list elements: ";
	cin >> n;
	if (n <= 0) return nullptr;

	cout << "Enter " << n << " elements: ";
	int val;
	cin >> val;

	ListN* head = new ListN(val);
	ListN* tail = head;

	for (int i = 1; i < n; ++i) {
		cin >> val;
		tail->next = new ListN(val);
		tail = tail->next;
	}

	return head;
}