#include "document/vocabulary.h"

#include <algorithm>
#include <ostream>
#include <vector>

#include "util/io.h"
#include "util/logging.h"

namespace deeplearning {
namespace embedding {

Vocabulary::~Vocabulary() {
  for (const Item* item : items_) {
    delete item;
  }
}

void Vocabulary::Build(size_t min_count) {
  CHECK(!has_built_) << "Vocabulary has already been built.";
  has_built_ = true;

  std::vector<Item*> to_remove_items;
  for (const auto& key_item : item_hash_) {
    Item* item = key_item.second;
    if (item->count() < min_count) {
      to_remove_items.push_back(item);
    }
    num_original_ += item->count();
  }

  size_t num_to_remove = 0;
  for (const Item* item : to_remove_items) {
    item_hash_.erase(item->text());
    num_to_remove += item->count();
    delete item;
  }

  num_retained_ = num_original_ - num_to_remove;

  for (const auto& key_item : item_hash_) {
    Item* item = key_item.second;
    items_.push_back(item);
  }
  std::sort(items_.begin(), items_.end(),
            [](const Item* item1, const Item* item2) {
              return item1->count() > item2->count();
            });
  for (size_t i = 0; i < items_.size(); ++i) {
    items_[i]->set_index(i);
  }
}

void Vocabulary::Write(std::ostream* out) const {
  CHECK(has_built_) << "Vocabulary has not been built.";
  util::WriteBasicItem(out, num_original_);
  util::WriteBasicItem(out, num_retained_);

  size_t items_size = items_.size();
  util::WriteBasicItem(out, items_size);
  for (const Item* item : items_) {
    item->Write(out);
  }
}

void Vocabulary::Clear() {
  for (const Item* item : items_) {
    delete item;
  }
  items_.clear();
  item_hash_.clear();
  has_built_ = false;
  num_original_ = 0;
  num_retained_ = 0;
}

std::string Vocabulary::ToString() const {
  std::string ans = "num_original=" + std::to_string(num_original_) +
                    ", num_retained=" + std::to_string(num_retained_) + "\n";
  for (const Item* item : items_) {
    ans += item->ToString() + "\n";
  }
  return ans;
}

bool Vocabulary::CheckEmpty() const {
  return num_original_ == 0 && num_retained_ == 0 && items_.empty() &&
         item_hash_.empty() && !has_built_;
}

void Vocabulary::Item::Write(std::ostream* out) const {
  util::WriteBasicItem(out, index_);
  util::WriteBasicItem(out, count_);
  util::WriteString(out, text_);
}

void Vocabulary::Item::Read(std::istream* in, Vocabulary::Item* item) {
  util::ReadBasicItem(in, &item->index_);
  util::ReadBasicItem(in, &item->count_);
  util::ReadString(in, &item->text_);
}

std::string Vocabulary::Item::ToString() const {
  return text_ + ":" + std::to_string(index_) + "," + std::to_string(count_);
}

}  // namespace embedding
}  // namespace deeplearning
