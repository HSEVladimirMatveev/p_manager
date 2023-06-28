#include <stdexcept>

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <iostream>
#include <cstring>
using std::cerr;
using std::endl;

#include "kuznechik.hpp"
#include "mycrypto.hpp"
#include "consts.hpp"

//8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef

bool Kuznyechik::is_init = false;
int TEST = 0;


/*
Arguments:
	const ByteBlock & src
	ByteBlock & dst
Return:
	None
Description:
	Конструктор класса.
*/
Kuznyechik::Kuznyechik(const ByteBlock & key) :
    keys(10) //итерационные ключи
{
    if(key.size() != 32){
        throw std::invalid_argument("Kuznyechik: The key must be 32 bytes long");
	}
    if(!is_init) {
        init_perms();
        init_consts();
        is_init = true;
    }
    keys[0].reset(key.byte_ptr(), BLOCK_LENGTH);
    keys[1].reset(key.byte_ptr() + BLOCK_LENGTH, BLOCK_LENGTH);
    for(int i = 0; i < 4; i++) {
        keys[2 * i + 2] = ByteBlock(BLOCK_LENGTH);
        keys[2 * i + 3] = ByteBlock(BLOCK_LENGTH);
        key_derivation128(
			keys[2 * i].byte_ptr(),
			keys[2 * i + 1].byte_ptr(),
			keys[2 * i + 2].byte_ptr(),
			keys[2 * i + 3].byte_ptr(),
			i
		);
    }
}
Kuznyechik::Kuznyechik(const Kuznyechik & rhs) {
	is_init = rhs.is_init;
	for(auto & iter_key : rhs.keys)
		keys.push_back(iter_key.deep_copy());
}
Kuznyechik::~Kuznyechik() {}

/*
Arguments:
	const ByteBlock & src
	ByteBlock & dst
Return:
	None
Description:
	Метод зашифрования.
*/
void Kuznyechik::encrypt(const ByteBlock & src, ByteBlock & dst) const {
    if(src.size() != BLOCK_LENGTH){
        throw std::invalid_argument("Блок должен быть из 16 байт!");
	}
    if(dst != src) dst = src.deep_copy();
    encrypt128(dst.byte_ptr(), keys);
}

/*
Arguments:
	const ByteBlock & src
	ByteBlock & dst
Return:
	None
Description:
	Метод расшифрования.
*/
void Kuznyechik::decrypt(const ByteBlock & src, ByteBlock & dst) const {
    if(src.size() != BLOCK_LENGTH)
        throw std::invalid_argument("Блок должен быть из 16 байт!");
    if(dst != src) dst = src.deep_copy();
    decrypt128(dst.byte_ptr(), keys);
}


void init_perms() {
	map<BYTE, BYTE> *p_direct, *p_inverse;
	p_direct = const_cast< map<BYTE, BYTE> * >(&direct_permutation);
	p_inverse = const_cast< map<BYTE, BYTE> * >(&inverse_permutation);
	for(int i = 0; i < nonlinear_transform_perm.size(); i++) {
		(*p_direct)[i] = nonlinear_transform_perm[i];
		(*p_inverse)[nonlinear_transform_perm[i]] = i;
	}
}

/*
Arguments:
	None
Return:
	None
Description:
	Инициализация констант.
*/
void init_consts() {
	vector<ByteBlock> * p = const_cast< vector<ByteBlock> * >(&iteration_constants);
    ByteBlock v128;
    for(BYTE i = 1; i <= 32; i++) {
        v128 = ByteBlock(BLOCK_LENGTH, 0);
		v128[BLOCK_LENGTH - 1] = i;
        iteration_linear_transform_direct128(v128.byte_ptr());
		p->push_back(std::move(v128));
	}
}

/*
Arguments:
	BYTE * dst, const BYTE * lhs, const BYTE * rhs
Return:
	None
Description:
	XOR для блока 128 бит.
*/
void xor128(BYTE * dst, const BYTE * lhs, const BYTE * rhs) {
	const BYTE * p_end = dst + BLOCK_LENGTH;
	while(dst != p_end) {
		*(dst++) = *(lhs++) ^ *(rhs++);
	}
}

/*
Arguments:
	WORD lhs
	WORD rhs
Return:
	WORD
Description:
	При выполнения линейного преобразования все вычисления ведутся в фактор-кольце GL(2)[x]/p(x),
	где p(x) = x^8 + x^7 + x^6 + x + 1.
	В первом цикле многочлены перемножаются, они заданны своими коэффициентами из GL(2).
	Во втором пошагово вычисляется значение по модулю p(x).
*/
WORD multiply(WORD lhs, WORD rhs) {
	WORD result = 0, modulus = linear_transform_modulus << 7;
	for(WORD detecter = 0x1; detecter != 0x100; detecter <<= 1, lhs <<= 1)
		if(rhs & detecter) result ^= lhs;
	for(WORD detecter = 0x8000; detecter != 0x80; detecter >>= 1, modulus >>= 1)
		if(result & detecter) result ^= modulus;
	return result;
}

/*
Arguments:
	BYTE * target;
Return:
	None
Description:
	Нелинейное прямое преобразование для блока 128 бит.
*/
void nonlinear_transform_direct128(BYTE * target) {
	BYTE * p_end = target + BLOCK_LENGTH;
	while(target != p_end) {
		*target = direct_permutation.at(*target);
		target++;
	}
}

/*
Arguments:
	BYTE * target;
Return:
	None
Description:
	Нелинейное обратное преобразование для блока 128 бит.
*/
void nonlinear_transform_inverse128(BYTE * target) {
	BYTE * p_end = target + BLOCK_LENGTH;
	while(target != p_end) {
		*target = inverse_permutation.at(*target);
		target++;
	}
}

/*
Arguments:
	const BYTE * target
Return:
	None
Description:
	Производится перемножение массивов char в поле Галуа.
*/
BYTE linear_transform_core128(const BYTE * target) {
	WORD result = 0;
	for(int i = 0; i < BLOCK_LENGTH; i++) {
        result ^= multiply(target[i], linear_transform_coeff[i]);
    }
	return result;
}

/*
Arguments:
	const BYTE * target
Return:
	None
Description:
	Линейное прямое преобразование для блока 128 бит.
*/
void linear_transform_direct128(BYTE * target) {
	BYTE buffer = linear_transform_core128(target);
	for(int i = BLOCK_LENGTH - 1; i > 0; i--)
		target[i] = target[i-1];
	*target = buffer;
}

/*
Arguments:
	const BYTE * target
Return:
	None
Description:
	Линейное обратное преобразование для блока 128 бит.
*/
void linear_transform_inverse128(BYTE * target)
{
	BYTE buffer = *target;
	for(int i = 0; i < BLOCK_LENGTH - 1; i++)
		target[i] = target[i+1];
	target[15] = buffer;
	target[15] = linear_transform_core128(target);
}


void iteration_linear_transform_direct128(BYTE * target)
{
    for(int i = 0; i < BLOCK_LENGTH; i++)
        linear_transform_direct128(target);
}

void iteration_linear_transform_inverse128(BYTE * target)
{
    for(int i = 0; i < BLOCK_LENGTH; i++)
        linear_transform_inverse128(target);
}
/*
Arguments:
	BYTE *
	const vector<ByteBlock> by reference
Return:
	None
Description:
	Функция шифрования для блока 128 бит.
*/
void encrypt128(BYTE * target, const vector<ByteBlock> & keys) {
	xor128(target, target, keys[0].byte_ptr());
	for(int i = 1; i < 10; i++) {
		nonlinear_transform_direct128(target);
		iteration_linear_transform_direct128(target);
		xor128(target, target, keys[i].byte_ptr());
	}
}

/*
Arguments:
	BYTE *
	const vector<ByteBlock> by reference
Return:
	None
Description:
	Функция дешифрования для блока 128 бит.
*/
void decrypt128(BYTE * target, const vector<ByteBlock> & keys) {
	xor128(target, target, keys[9].byte_ptr());
	for(int i = 8; i >= 0; i--) {
		iteration_linear_transform_inverse128(target);
        nonlinear_transform_inverse128(target);
        xor128(target, target, keys[i].byte_ptr());
	}
}

/*
Arguments:
	BYTE * k1
	BYTE * k2
	int iconst
Return:
	None
Description:
	Трансформация ключей для блока 128 бит.
*/
void keys_transform128(BYTE * k1, BYTE * k2, int iconst) {
	BYTE buffer[BLOCK_LENGTH];
	memcpy(buffer, k1, BLOCK_LENGTH);
	xor128(k1, k1, iteration_constants[iconst].byte_ptr());
	nonlinear_transform_direct128(k1);
	iteration_linear_transform_direct128(k1);
	xor128(k1, k2, k1);
	memcpy(k2, buffer, BLOCK_LENGTH);
}

/*
Arguments:
	BYTE * k1
	BYTE * k2
	BYTE * k3
	BYTE * k4
	int ipair
Return:
	None
Description:
	Развёртывание итерационных ключей для блока 128 бит.
*/
void key_derivation128(BYTE * k1, BYTE * k2, BYTE * k3, BYTE * k4, int ipair) {
	if(k1 != k3) memcpy(k3, k1, BLOCK_LENGTH);
	if(k2 != k4) memcpy(k4, k2, BLOCK_LENGTH);
	for(int i = 0; i < 8; i++) {
		keys_transform128(k3, k4, ipair * 8 + i);
	}
}


int main()
{
	return 1;
}
