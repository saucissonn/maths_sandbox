class Tree:
    def __init__(self, root, left, right):
        self.root = root
        self.right = right
        self.left = left


class queue:
    def __init__(self):
        self.list = []

    def enqueue(self, v):
        self.list.append(v)

    def dequeue(self):
        self.list.pop(0)
