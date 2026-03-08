#ifndef DIFFY_HELLMAN_IMPLEMENTATION_DIFFYHELLMAN128_H
#define DIFFY_HELLMAN_IMPLEMENTATION_DIFFYHELLMAN128_H

#include <cstdint>
#include <array>
#include <vector>
#include <bit>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef std::array<u64,2> u128;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

//
// -------- u128 Lib --------
//

/// a > b
inline bool greaterThan(const u128 a, const u128 b) {
    if (a[1] > b[1]) {
        return true;
    }
    if (a[1] == b[1]) {
        if (a[0] > b[0]) {
            return true;
        }
        return false;
    }
    return false;
}

/// a < b
inline bool lesserThan(const u128 a, const u128 b) {
    if (a[1] < b[1]) {
        return true;
    }
    if (a[1] == b[1]) {
        if (a[0] < b[0]) {
            return true;
        }
        return false;
    }
    return false;
}

/// a == b
inline bool equal(const u128 a, const u128 b) {
    if (a[1]==b[1] && a[0]==b[0]) {
        return true;
    }
    return false;
}

/// a == (u64)b
inline bool equal(const u128 a, const u64 b) {
    if (a[1] == 0ull && a[0]==b) {
        return true;
    }
    return false;
}

/// a << n
inline u128 shiftLeft(const u128 a, const u8 n) {
    u128 res = a;
    for (i32 i = 0; i < n; i++) {
        res[1] <<= 1;
        res[1] ^= res[0] >> 63;
        res[0] <<= 1;
    }
    return res;
}

/// a >> n
inline u128 shiftRight(const u128 a, const u8 n) {
    u128 res = a;
    for (i32 i = 0; i < n; i++) {
        res[0] >>= 1;
        res[0] ^= (res[1] & 1) << 63;
        res[1] >>= 1;
    }
    return res;
}

/// a + b
inline u128 add(const u128 a, const u128 b) {
    u128 res = a;
    if (b[0] + res[0] < b[0]) {
        res[1] += 1;
    }
    res[0] += b[0];
    res[1] += b[1];
    return res;
}

/// a * b
inline u128 multiply(const u128 a, const u128 b) {
    u128 res{};
    u128 A = a;
    u128 B = b;
    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            res = add(res, A);
        }
        A = shiftLeft(A, 1);
        B = shiftRight(B, 1);
    }
    return res;
}

/// a * (u64)b
inline u128 multiply(const u128 a, const u64 b) {
    u128 res{};
    u128 A = a;
    u128 B = {b,0};
    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            res = add(res, A);
        }
        A = shiftLeft(A, 1);
        B = shiftRight(B, 1);
    }
    return res;
}


//
// -------- Polynomials Lib --------
//

inline std::vector<u128> polyTable;

inline void init128(u128 poly) {
    polyTable.emplace_back(poly);
    for (i32 i = 1; i < 128; i++) {
        u128 newVal = polyTable[i-1];
        bool carry = false;
        bool overflow = false;
        if (newVal[0] & (1ull << 63)) {
            carry = true;
        }
        newVal[0] <<= 1;
        if (newVal[1] & (1ull << 63)) {
            overflow = true;
        }
        newVal[1] <<= 1;
        if (carry) {
            newVal[1] ^= 1;
        }
        if (overflow) {
            newVal[0] ^= poly[0];
            newVal[1] ^= poly[1];
        }
        polyTable.emplace_back(newVal);
    }
}

inline u128 addPolynomial(const u128 a, const u128 b) {
    u128 res = a;
    res[0] ^= b[0];
    res[1] ^= b[1];
    return res;
}

inline u128 multiplyPolynomial(const u64 a, const u64 b) {
    u128 result{};
    u128 overflow{};
    u128 A = {a,0};
    u128 B = {b,0};
    u128 a_overflow{};

    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            result[0] ^= A[0];
            result[1] ^= A[1];
            overflow[0] ^= a_overflow[0];
            overflow[1] ^= a_overflow[1];
        }
        a_overflow[1] <<= 1;
        a_overflow[1] ^= a_overflow[0] >> 63;
        a_overflow[0] <<= 1;
        a_overflow[0] ^= A[1] >> 63;
        A[1] <<= 1;
        A[1] ^= A[0] >> 63;
        A[0] <<= 1;
        B[0] >>= 1;
        B[0] ^= (B[1] & 1) << 63;
        B[1] >>= 1;
    }

    for (i32 i = 0; i < polyTable.size(); i++) {
        if (overflow[0] & 1) {
            result[0] ^= polyTable[i][0];
            result[1] ^= polyTable[i][1];
        }
        overflow[0] >>= 1;
        overflow[0] ^= (overflow[1] & 1) << 63;
        overflow[1] >>= 1;
    }

    return result;
}

inline u128 multiplyPolynomial(const u128 a, const u64 b) {
    u128 result{};
    u128 overflow{};
    u128 A = a;
    u128 B = {b,0};
    u128 a_overflow{};

    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            result[0] ^= A[0];
            result[1] ^= A[1];
            overflow[0] ^= a_overflow[0];
            overflow[1] ^= a_overflow[1];
        }
        a_overflow[1] <<= 1;
        a_overflow[1] ^= a_overflow[0] >> 63;
        a_overflow[0] <<= 1;
        a_overflow[0] ^= A[1] >> 63;
        A[1] <<= 1;
        A[1] ^= A[0] >> 63;
        A[0] <<= 1;
        B[0] >>= 1;
        B[0] ^= (B[1] & 1) << 63;
        B[1] >>= 1;
    }

    for (i32 i = 0; i < polyTable.size(); i++) {
        if (overflow[0] & 1) {
            result[0] ^= polyTable[i][0];
            result[1] ^= polyTable[i][1];
        }
        overflow[0] >>= 1;
        overflow[0] ^= (overflow[1] & 1) << 63;
        overflow[1] >>= 1;
    }

    return result;
}

inline u128 multiplyPolynomial(const u64 a, const u128 b) {
    u128 result{};
    u128 overflow{};
    u128 A = {a,0};
    u128 B = b;
    u128 a_overflow{};

    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            result[0] ^= A[0];
            result[1] ^= A[1];
            overflow[0] ^= a_overflow[0];
            overflow[1] ^= a_overflow[1];
        }
        a_overflow[1] <<= 1;
        a_overflow[1] ^= a_overflow[0] >> 63;
        a_overflow[0] <<= 1;
        a_overflow[0] ^= A[1] >> 63;
        A[1] <<= 1;
        A[1] ^= A[0] >> 63;
        A[0] <<= 1;
        B[0] >>= 1;
        B[0] ^= (B[1] & 1) << 63;
        B[1] >>= 1;
    }

    for (i32 i = 0; i < polyTable.size(); i++) {
        if (overflow[0] & 1) {
            result[0] ^= polyTable[i][0];
            result[1] ^= polyTable[i][1];
        }
        overflow[0] >>= 1;
        overflow[0] ^= (overflow[1] & 1) << 63;
        overflow[1] >>= 1;
    }

    return result;
}

inline u128 multiplyPolynomial(const u128 a, const u128 b) {
    u128 result{};
    u128 overflow{};
    u128 A = a;
    u128 B = b;
    u128 a_overflow{};

    for (i32 i = 0; i < 128; i++) {
        if (!B[0] && !B[1]) {
            break;
        }
        if (B[0] & 1) {
            result[0] ^= A[0];
            result[1] ^= A[1];
            overflow[0] ^= a_overflow[0];
            overflow[1] ^= a_overflow[1];
        }
        a_overflow[1] <<= 1;
        a_overflow[1] ^= a_overflow[0] >> 63;
        a_overflow[0] <<= 1;
        a_overflow[0] ^= A[1] >> 63;
        A[1] <<= 1;
        A[1] ^= A[0] >> 63;
        A[0] <<= 1;
        B[0] >>= 1;
        B[0] ^= (B[1] & 1) << 63;
        B[1] >>= 1;
    }

    for (i32 i = 0; i < polyTable.size(); i++) {
        if (overflow[0] & 1) {
            result[0] ^= polyTable[i][0];
            result[1] ^= polyTable[i][1];
        }
        overflow[0] >>= 1;
        overflow[0] ^= (overflow[1] & 1) << 63;
        overflow[1] >>= 1;
    }

    return result;
}

inline u128 fastPolyPower(const u128 a, const u64 e) {
    u128 result{1,0};
    u128 A = a;

    const u8 m = std::bit_width(e);

    for (i32 i = 0; i < m; i++) {
        if ((e >> i) & 1) {
            result = multiplyPolynomial(result, A);
        }
        A = multiplyPolynomial(A, A);
    }
    return result;
}

inline u128 fastPolyPower(const u128 a, const u128 e) {
    u128 result{1,0};
    u128 A = a;
    u128 E = e;

    u8 m;
    if (E[1] == 0) {
        m = std::bit_width(E[0]);
    } else {
        m = std::bit_width(E[1]) + 64;
    }

    for (i32 i = 0; i < m; i++) {
        if (E[0] & 1) {
            result = multiplyPolynomial(result, A);
        }
        A = multiplyPolynomial(A, A);
        E = shiftRight(E, 1);
    }

    return result;
}

//
// -------- Diffy-Hellman Functions --------
//

/// calculates public key from provided generator and private key
inline u128 getPublicKey(const u128 generator, const u128 privateKey) {
    return fastPolyPower(generator, privateKey);
}

/// calculates communication key from private key and received public key
inline u128 getCommunicationKey(const u128 privateKey, const u128 publicKey) {
    return fastPolyPower(publicKey, privateKey);
}

#endif //DIFFY_HELLMAN_IMPLEMENTATION_DIFFYHELLMAN128_H