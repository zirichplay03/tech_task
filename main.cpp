#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

/**
 * @brief Узел двусвязного списка с дополнительным случайным указателем.
 */
struct ListNode {
    ListNode* prev = nullptr;  ///< Указатель на предыдущий элемент
    ListNode* next = nullptr;  ///< Указатель на следующий элемент
    ListNode* rand = nullptr;  ///< Указатель на случайный элемент списка
    std::string data;          ///< Данные узла
};

/**
 * @brief Временная структура для хранения данных из входного файла.
 */
struct TempNode {
    std::string data; ///< Строковые данные
    int rand_index;   ///< Индекс случайного узла
};

/**
 * @brief Читает входной файл и формирует временный массив узлов.
 *
 * Формат строки: "data;rand_index"
 *
 * @param filename Имя входного файла
 * @return std::vector<TempNode> Вектор временных узлов
 * @throw std::runtime_error если файл не открыт или формат строки некорректен
 */
std::vector<TempNode> readInput(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Cannot open input file");
    }

    std::vector<TempNode> nodes;
    std::string line;

    while (std::getline(in, line)) {
        size_t pos = line.rfind(';');
        if (pos == std::string::npos) {
            throw std::runtime_error("Invalid line format");
        }

        std::string data = line.substr(0, pos);
        int rand_index = std::stoi(line.substr(pos + 1));

        nodes.push_back({data, rand_index});
    }

    return nodes;
}

/**
 * @brief Строит двусвязный список из временного массива.
 *
 * @param temp Вектор временных узлов
 * @return ListNode* Указатель на голову списка
 */
ListNode* buildList(const std::vector<TempNode>& temp) {
    if (temp.empty()) return nullptr;

    size_t n = temp.size();
    std::vector<ListNode*> nodes(n);

    /// Создание узлов
    for (size_t i = 0; i < n; ++i) {
        nodes[i] = new ListNode();
        nodes[i]->data = temp[i].data;
    }

    /// Связывание prev/next
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) nodes[i]->prev = nodes[i - 1];
        if (i + 1 < n) nodes[i]->next = nodes[i + 1];
    }

    /// Установка случайных указателей
    for (size_t i = 0; i < n; ++i) {
        int ri = temp[i].rand_index;
        if (ri >= 0 && ri < (int)n) {
            nodes[i]->rand = nodes[ri];
        }
    }

    return nodes[0];
}

/**
 * @brief Сериализует список в бинарный файл.
 *
 * Формат:
 * - количество узлов
 * - для каждого узла:
 *   - длина строки
 *   - строка
 *   - индекс rand
 *
 * @param head Указатель на голову списка
 * @param filename Имя выходного файла
 * @throw std::runtime_error если файл не открыт
 */
void serialize(ListNode* head, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot open output file");
    }

    /// Сбор узлов в массив
    std::vector<ListNode*> nodes;
    for (ListNode* cur = head; cur != nullptr; cur = cur->next) {
        nodes.push_back(cur);
    }

    size_t size = nodes.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(size));

    /// Создание отображения узел -> индекс
    std::unordered_map<ListNode*, int> indexMap;
    for (size_t i = 0; i < size; ++i) {
        indexMap[nodes[i]] = (int)i;
    }

    /// Запись данных
    for (size_t i = 0; i < size; ++i) {
        ListNode* node = nodes[i];

        /// Запись строки
        size_t len = node->data.size();
        out.write(reinterpret_cast<char*>(&len), sizeof(len));
        out.write(node->data.data(), len);

        /// Запись индекса случайного узла
        int rand_index = -1;
        if (node->rand) {
            rand_index = indexMap[node->rand];
        }

        out.write(reinterpret_cast<char*>(&rand_index), sizeof(rand_index));
    }
}

/**
 * @brief Десериализует список из бинарного файла.
 *
 * @param filename Имя файла
 * @return ListNode* Указатель на восстановленный список
 * @throw std::runtime_error если файл не открыт
 */
ListNode* deserialize(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file for reading");
    }

    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    if (size == 0) return nullptr;

    std::vector<ListNode*> nodes(size);

    /// Создание узлов
    for (size_t i = 0; i < size; ++i) {
        nodes[i] = new ListNode();
    }

    /// Чтение данных
    for (size_t i = 0; i < size; ++i) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));

        std::string data(len, '\0');
        in.read(&data[0], len);
        nodes[i]->data = data;

        int rand_index;
        in.read(reinterpret_cast<char*>(&rand_index), sizeof(rand_index));

        if (rand_index >= 0 && rand_index < (int)size) {
            nodes[i]->rand = nodes[rand_index];
        }
    }

    /// Восстановление связей prev/next
    for (size_t i = 0; i < size; ++i) {
        if (i > 0) nodes[i]->prev = nodes[i - 1];
        if (i + 1 < size) nodes[i]->next = nodes[i + 1];
    }

    return nodes[0];
}

/**
 * @brief Освобождает память, занятую списком.
 *
 * @param head Указатель на голову списка
 */
void freeList(ListNode* head) {
    while (head) {
        ListNode* next = head->next;
        delete head;
        head = next;
    }
}

/**
 * @brief Точка входа в программу.
 *
 * Последовательность:
 * 1. Чтение входного файла
 * 2. Построение списка
 * 3. Сериализация
 * 4. Десериализация (проверка)
 * 5. Вывод результата
 * 6. Освобождение памяти
 */
int main() {
    try {
        auto temp = readInput("inlet.in");
        std::cout << "Loaded nodes: " << temp.size() << "\n";

        ListNode* head = buildList(temp);

        serialize(head, "outlet.out");
        std::cout << "Serialized to outlet.out\n";

        ListNode* restored = deserialize("outlet.out");
        std::cout << "Deserialized list:\n";

        for (ListNode* cur = restored; cur; cur = cur->next) {
            std::cout << cur->data;

            if (cur->rand) {
                std::cout << " -> rand: " << cur->rand->data;
            } else {
                std::cout << " -> rand: null";
            }

            std::cout << "\n";
        }

        freeList(head);
        freeList(restored);

        std::cout << "DONE\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}