#include <string>
using std::string;
#include <vector>
using std::vector;
#ifndef __MYCRYPTO__
#define __MYCRYPTO__

typedef unsigned char BYTE;
typedef unsigned short WORD;
/*
Класс блока байт.
*/
class ByteBlock {
	BYTE * pBlocks;
	size_t amount_of_bytes;
public:
    /*
    Конструктор создаёт блок байтов из массива char. 
    */
	ByteBlock(size_t size_ = 0, BYTE init_value = 0);

    /*
    Конструктор копирования.
    */
    ByteBlock(const BYTE * pBlocks_, size_t size_);

    /*
    Конструктор перемещения, копирует и удаляет ориганал.
    */
    ByteBlock(ByteBlock && rhs);

    /*
    Деструктор.
    */
    ~ByteBlock();

    // Move assigment operator
    // Object to move turn to null
	void operator = (ByteBlock && rhs);

    BYTE * byte_ptr();
    const BYTE * byte_ptr() const;

    // Indexing operator with evident functionality
    BYTE & operator [] (size_t index);
    BYTE operator [] (size_t index) const;

	bool operator == (const ByteBlock & lhs) const;
	bool operator != (const ByteBlock & lhs) const;

    /*
    Замена тела блока
    Старое значение будет равно нулю, а затем удалено
    Новое значение копируется в блок,
    источник остается неприкосновенным
    */
    void reset(const BYTE * pBlocks_, size_t size_);

    // Возвращает количество байт в блоке
    size_t size() const;

    // Возвращает deep copy блока, которая указывает на другое место в памяти
    ByteBlock deep_copy() const;

	// Возвращает фрагмент текущего байтового блока
    ByteBlock operator () (size_t begin, size_t length) const;

	// Изменяет значения между двумя байтовыми блоками
	friend void swap(ByteBlock & lhs, ByteBlock & rhs);
};

std::vector<ByteBlock> split_blocks(const ByteBlock & src, size_t length);
ByteBlock join_blocks(const std::vector<ByteBlock> & blocks);
void xor_blocks(ByteBlock & to_assign, const ByteBlock & lhs, const ByteBlock & rhs);

string hex_representation(const ByteBlock & bb);
ByteBlock hex_to_bytes(const string & s);


/*
Description:
    Шаблонный класс, обеспечивающий реализацию режима шифрования CFB.
    Т.е. каждый блок сообщения шифруется отдельно, а далее "склеивается" шифротекст.
    Содержит: 
        конструктор копирования,
        методы шифрования и дешифрования с одним и тем же интерфейсом,
        общедоступный элемент, равный длине блока данных.
*/
template <typename CipherType>
class CFB_Mode {
    const CipherType algorithm;
    const ByteBlock iv;

	void decrypt_with_iv(const ByteBlock & src, ByteBlock & dst, const ByteBlock & iv_) const;
public:
    CFB_Mode(const CipherType & alg, const ByteBlock & init_vec);
    void encrypt(const ByteBlock & src, ByteBlock & dst) const;
    void decrypt(const ByteBlock & src, ByteBlock & dst) const;
};
#endif