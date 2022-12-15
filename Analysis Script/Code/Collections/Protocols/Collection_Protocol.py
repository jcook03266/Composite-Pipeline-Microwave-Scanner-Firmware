from typing import Protocol
from enum import Enum


class GenericCollection(Protocol):
    id: int = 0
    collection: list
    count: int
    max_count: int = None

    class ErrorCodes(Enum):
        unknown = "Error: An unknown error has occurred"
        item_not_found = "Error: This item is not present in this collection"
        empty_collection = "Error: This collection contains no scans"
        item_limit_reached = "Warning: The limit of this collection has been reached, and no more items can be added"

        def display_error(self, code):
            switch = {
                self.unknown: self.unknown,
                self.item_not_found: self.item_not_found,
                self.empty_collection: self.empty_collection,
                self.item_limit_reached: self.item_limit_reached
            }

            if switch.get(code) is not None:
                print(switch.get(code))

    def add(self, item):
        if not self.is_full():
            self.collection.append(item)
        else:
            self.ErrorCodes.display_error(self.ErrorCodes.item_limit_reached)

    def remove(self, item):
        if not self.collection.__contains__(item):
            self.ErrorCodes.display_error(self.ErrorCodes.item_not_found)
            return
        elif self.is_empty():
            self.ErrorCodes.display_error(self.ErrorCodes.empty_collection)
            return

        self.collection.remove(item)

    def update_collection(self,
                          new_collection: list):
        if len(new_collection) > self.max_count:
            return

        self.collection = new_collection
        self.update_item_count()

    def is_empty(self) -> bool:
        return len(self.collection) == 0

    def is_full(self) -> bool:
        if self.max_count is None:
            return False

        return self.max_count == self.get_item_count()

    def update_item_count(self):
        self.count = len(self.collection)

    def get_item_count(self) -> int:
        return len(self.collection)

    def get_collection_copy(self) -> list:
        return self.collection.copy()

    def clear(self):
        self.collection.clear()
