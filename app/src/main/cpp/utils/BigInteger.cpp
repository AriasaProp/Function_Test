#include <cmath>
#include <algorithm>
#include "utils/BigInteger.h"

static const size_t WORD_BITS = sizeof(word) * CHAR_BIT;
static const size_t WORD_BITS_1 = WORD_BITS - 1;
static const word WORD_MASK = (word)-1;
static const size_t WORD_HALF_BITS = sizeof(word) * CHAR_BIT / 2;
static const word WORD_HALF_MASK = WORD_MASK >> WORD_HALF_BITS;
static const double LOG2BITS = std::log(2) * WORD_BITS;

//Constructors
BigInteger::BigInteger() : neg(false) {}
BigInteger::BigInteger(const BigInteger &a) : neg(a.neg)
{
    words = a.words;
}
BigInteger::BigInteger(const signed &i) : neg(i < 0)
{
    unsigned u = abs(i);
    while (u)
    {
        words.push_back(u);
        size_t i = WORD_BITS;
        while (i--)
            u >>= 1;
    }
}

// in constructor should begin with sign or number, add '\0' in last world to close number
BigInteger::BigInteger(const char *C) : neg(false)
{
    const char *c = C;
    // read sign
    if (*c == '-')
        neg = true, c++;
    // read digits
    for (; *c; c++)
    {
        word b = *c - '0';
        if (b >= 10)
            throw("BigInteger should initialize with number from 0 to 9.");
        word carry = 0, tmp, a_hi, a_lo, tp;
        size_t i;
        for (i = 0; i < words.size(); i++)
        {
            tmp = this->words[i] * 10;
            carry = ((tmp += carry) < carry);
            a_hi = this->words[i] >> WORD_HALF_BITS;
            a_lo = this->words[i] & WORD_HALF_MASK;
            tp = ((a_lo * 10) >> WORD_HALF_BITS) + a_hi * 10;
            carry += (tp >> WORD_HALF_BITS) + ((tp & WORD_HALF_MASK) >> WORD_HALF_BITS);
            this->words[i] = tmp;
        }
        if (carry)
            words.push_back(carry);
        for (i = 0; i < words.size() && b; i++)
            b = (this->words[i] += b) < b;
        if (b)
            words.push_back(b);
    }
    while (words.size() && !words.back())
        words.pop_back();
}

// Destructors
BigInteger::~BigInteger()
{
    words.clear();
}

word BigInteger::get_bit(size_t i) const
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (i_word >= words.size())
        return 0;
    return (this->words[i_word] >> i_bit) & 1;
}

void BigInteger::clr_bit(size_t i)
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (i_word >= words.size())
        return;
    this->words[i_word] &= ~(1 << i_bit);
}

void BigInteger::set_bit(size_t i)
{
    const size_t i_word = i / WORD_BITS, i_bit = i % WORD_BITS;
    if (words.size() <= i_word)
        words.resize(i_word + 1);
    this->words[i_word] |= ((word)1) << i_bit;
}

size_t BigInteger::tot() const
{
    return words.size() * WORD_BITS;
}

char *BigInteger::to_chars() const
{
    std::vector<char> text;
    std::vector<word> A = this->words;
    text.reserve(LOG2BITS * A.size() + 2);
    word remainder, current;
    size_t i;
    while (A.size())
    {
        remainder = 0;
        for (i = A.size(); i--;)
        {
            current = A[i];
            A[i] = 0;
            //part 1
            remainder <<= WORD_HALF_BITS;
            remainder |= current >> WORD_HALF_BITS;
            A[i] <<= WORD_HALF_BITS;
            A[i] |= remainder / 10;
            remainder %= 10;
            //part 2
            remainder <<= WORD_HALF_BITS;
            remainder |= current & WORD_HALF_MASK;
            A[i] <<= WORD_HALF_BITS;
            A[i] |= remainder / 10;
            remainder %= 10;
        }
        while (A.size() && !A.back())
            A.pop_back();
        text.push_back('0' + char(remainder));
    }
    if (this->neg)
        text.push_back('-');
    std::reverse(text.begin(), text.end());
    text.push_back('\0');
    char *result = new char[text.size()];
    std::copy(text.begin(), text.end(), result);
    return result;
}

double BigInteger::to_double() const
{
    if (!words.size())
        return 0.0;
    double d = 0.0, base = std::pow(2.0, WORD_BITS);
    for (size_t i = words.size(); i--;)
        d = d * base + this->words[i];
    return neg ? -d : d;
}

bool BigInteger::can_convert_to_int(int *result) const
{
    if (words.size())
    {
        if (words.size() > 1 || this->words[0] >= (WORD_MASK >> 1))
            return false;
        *result = this->words[0];
        if (neg)
            *result = -(*result);
    }
    else
        *result = 0;
    return true;
}

BigInteger BigInteger::mod_pow(BigInteger exponent, const BigInteger &modulus) const
{
    BigInteger result(1), base = (*this) % modulus;
    for (; exponent.words.size() > 0; exponent >>= 1)
    {
        if (exponent.words[0] & 1)
            result = (result * base) % modulus;
        base = (base * base) % modulus;
    }
    return result;
}

BigInteger BigInteger::sqrt() const
{
    BigInteger n = *this;
    size_t bit;
    if (!this->words.size())
        bit = 0;
    else
    {
        bit = (this->words.size() - 1) * WORD_BITS;
        for (word a = words.back(); a; a >>= 1)
            bit++;
        if (bit & 1)
            bit ^= 1;
    }
    BigInteger result;
    for (; bit >= 0; bit -= 2)
    {
        BigInteger tmp = result;
        tmp.set_bit(bit);
        if (n >= tmp)
        {
            n -= tmp;
            result.set_bit(bit + 1);
        }
        result >>= 1;
    }
    return result;
}
BigInteger &BigInteger::operator=(const signed &a)
{
    this->neg = (a < 0);
    if (this->words.size())
        this->words.clear();
    unsigned u = abs(a);
    size_t i;
    while (u)
    {
        this->words.push_back(u);
        i = WORD_BITS;
        while (i--)
            u >>= 1;
    }
    return *this;
}
BigInteger &BigInteger::operator=(const BigInteger &a)
{
    this->words = a.words;
    this->neg = a.neg;
    return *this;
}

BigInteger &BigInteger::operator--()
{
    word carry0 = 1;
    size_t i;
    const size_t n = words.size();
    if (this->neg)
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = ((this->words[i] += carry0) < carry0);
        if (carry0)
            this->words.push_back(carry0);
    }
    else
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = this->words[i] < (this->words[i] -= carry0);
        while (words.size() && !words.back())
            words.pop_back();
    }
    return *this;
}
BigInteger &BigInteger::operator++()
{
    word carry0 = 1;
    size_t i;
    const size_t n = words.size();
    if (this->neg)
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = this->words[i] < (this->words[i] -= carry0);
        while (words.size() && !words.back())
            words.pop_back();
    }
    else
    {
        for (i = 0; i < n && carry0; i++)
            carry0 = ((this->words[i] += carry0) < carry0);
        if (carry0)
            this->words.push_back(carry0);
    }
    return *this;
}

BigInteger &BigInteger::operator+=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> &A = this->words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (this->neg == b.neg)
    {
        const size_t n = na >= nb ? na : nb;
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = ((A[i] += carry0) < carry0);
            carry0 += ((A[i] += B[i]) < B[i]);
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                this->neg = !this->neg;
                na = A.size(), nb = B.size();
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        this->neg = !this->neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return *this;
}

BigInteger &BigInteger::operator-=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> &A = this->words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (this->neg != b.neg)
    {
        const size_t n = na >= nb ? na : nb;
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = (A[i] += carry0) < carry0;
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                this->neg = !this->neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        this->neg = !this->neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &b)
{
    const size_t nb = b.words.size();
    if (!nb)
    {
        this->words.clear();
        this->neg = false;
    }
    const size_t na = this->words.size();
    if (!na)
        return *this;
    this->neg ^= b.neg;
    if ((na >= nb ? na : nb) > 20)
    {
        const std::vector<word> &A = this->words, &B = b.words;
	      BigInteger result, a0, a1, b0, b1;
        const size_t m2 = (na >= nb) ? (na / 2 + (na & 1)) : (nb / 2 + (nb & 1));
        const bool hA = na > m2, hB = nb > m2;
        if (hA)
        {
            auto a_split = std::next(A.cbegin(), m2);
            a0.words = std::vector<word>(A.cbegin(), a_split);
	          while (a0.words.size() && !a0.words.back())
     	          a0.words.pop_back();
            a1.words = std::vector<word>(a_split, A.cend());
        }
        else
            a0.words = A;
        if (hB)
        {
            auto b_split = std::next(B.cbegin(), m2);
            b0.words = std::vector<word>(B.cbegin(), b_split);
	          while (b0.words.size() && !b0.words.back())
                b0.words.pop_back();
            b1.words = std::vector<word>(b_split, B.cend());
        }
        else
            b0.words = B;
        const BigInteger z0 = a0 * b0;
        if (hA && hB)
        {
            const BigInteger z1 = a1 * b1;
            result = z1;
            result.words.insert(result.words.begin(), m2, 0);
            result += (a1 + a0) * (b0 + b1) - z1 - z0;
        }
        else
            result = (a1 + a0) * (b0 + b1) - z0;
        result.words.insert(result.words.begin(), m2, 0);
        result += z0;
        this->words = result.words;
    }
    else
    {
        const std::vector<word> A = this->words, B = b.words;
        this->words.resize(na + nb + 1, 0);
        std::fill(this->words.begin(), this->words.end(), 0);
        word carry[2], aw[2], bw[2]; // temporary value
        for (size_t ia = 0, ib; ia < na; ia++)
        {
            aw[1] = A[ia] >> WORD_HALF_BITS;
            aw[0] = A[ia] & WORD_HALF_MASK;
            for (ib = 0; ib < nb; ib++)
            {
                auto i = this->words.begin() + ia + ib;
                auto j = this->words.end();
                bw[1] = B[ib] >> WORD_HALF_BITS;
                bw[0] = B[ib] & WORD_HALF_MASK;
                carry[0] = A[ia] * B[ib];
                carry[1] = ((aw[0] * bw[0]) >> WORD_HALF_BITS) + aw[1] * bw[0];
                carry[0] = (*(i++) += carry[0]) < carry[0];
                carry[1] = (carry[1] >> WORD_HALF_BITS) + ((aw[0] * bw[1] + (carry[1] & WORD_HALF_MASK)) >> WORD_HALF_BITS) + aw[1] * bw[1];
                carry[0] = ((*i += carry[0]) < carry[0]) + ((*(i++) += carry[1]) < carry[1]);
                while (carry[0] && (i != j))
                    carry[0] = (*(i++) += carry[0]) < carry[0];
                if (carry[0])
                    this->words.push_back(carry[0]);
            }
        }
    }
    while (this->words.size() && !this->words.back())
        this->words.pop_back();
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &b)
{
    word carry0 = 0, carry1;
    std::vector<word> rem = this->words, div = b.words;
    this->words.clear();
    //compare function
    bool cmp = true;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        this->neg ^= b.neg;
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            for (i = nb - 1; i--;)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        this->words.reserve(n_words + 1);
        do
        {
            //compare function
            cmp = true;
            na = rem.size(), nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                }
                while (rem.size() && !rem.back())
                    rem.pop_back();
                set_bit(n);
            }
            //reverse shift one by one
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
    }
    return *this;
}
BigInteger &BigInteger::operator%=(const BigInteger &b)
{
    //compare function
    bool cmp = true;
    word carry0, carry1;
    std::vector<word> &rem = this->words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            for (i = nb - 1; i--;)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        do
        {
            //compare function
            cmp = true;
            na = rem.size(), nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                    carry0 = rem[i] < (rem[i] -= carry0);
                while (rem.size() && !rem.back())
                    rem.pop_back();
            }
            //reverse shift one by one
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
    }
    return *this;
}

BigInteger &BigInteger::operator^=(size_t exponent)
{
    BigInteger p = *this;
    *this = 1;
    for (; exponent; exponent >>= 1)
    {
        if (exponent & 1)
        {
            *this *= p;
            exponent--;
        }
        p *= p;
    }
    return *this;
}

bool BigInteger::operator==(const BigInteger &b) const
{
    if (this->neg != b.neg || this->words.size() != b.words.size())
        return false;
    for (size_t i = this->words.size(); i--;)
        if (this->words[i] != b.words[i])
            return false;
    return true;
}
bool BigInteger::operator!=(const BigInteger &b) const
{
    if (this->neg != b.neg || this->words.size() != b.words.size())
        return true;
    for (size_t i = this->words.size(); i--;)
        if (this->words[i] != b.words[i])
            return true;
    return false;
}
bool BigInteger::operator<=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i > nb : i < nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A > B : A < B;
    }
    return true;
}
bool BigInteger::operator>=(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return this->neg ? i < nb : i > nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A < B : A > B;
    }
    return true;
}
bool BigInteger::operator<(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i > nb : i < nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A > B : A < B;
    }
    return false;
}
bool BigInteger::operator>(const BigInteger &b) const
{
    if (this->neg != b.neg)
        return !this->neg;
    size_t i = this->words.size(), nb = b.words.size();
    if (i != nb)
        return (this->neg) ? i < nb : i > nb;
    word A, B;
    while (i--)
    {
        A = this->words[i];
        B = b.words[i];
        if (A != B)
            return (this->neg) ? A < B : A > B;
    }
    return false;
}

BigInteger BigInteger::operator+(const BigInteger &b) const
{
    BigInteger r(*this);
    word carry0 = 0, carry1;
    std::vector<word> &A = r.words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    if (r.neg == b.neg)
    {
        const size_t n = std::max(na, nb);
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                r.neg = !r.neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        r.neg = !r.neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return r;
}

BigInteger BigInteger::operator-(const BigInteger &b) const
{
    BigInteger r(*this);
    std::vector<word> &A = r.words, B = b.words;
    size_t na = A.size(), nb = B.size(), i;
    word carry0 = 0, carry1 = 0;
    if (this->neg != b.neg)
    {
        const size_t n = std::max(na, nb);
        A.resize(n);
        for (i = 0; i < nb; i++)
        {
            carry0 = (A[i] += carry0) < carry0;
            carry0 += (A[i] += B[i]) < B[i];
        }
        for (; i < n && carry0; i++)
            carry0 = ((A[i] += carry0) < carry0);
        if (carry0)
            A.push_back(carry0);
    }
    else
    {
        if (na != nb)
        {
            if (na < nb)
            {
                A.swap(B);
                na = A.size(), nb = B.size();
                r.neg = !r.neg;
            }
        }
        else
        {
            i = na;
            while (i--)
            {
                carry0 = A[i], carry1 = B[i];
                if (carry0 != carry1)
                {
                    if (carry0 < carry1)
                    {
                        A.swap(B);
                        r.neg = !r.neg;
                    }
                    break;
                }
            }
        }
        carry0 = 0;
        for (i = 0; i < nb; i++)
        {
            carry0 = A[i] < (A[i] -= carry0);
            carry0 += A[i] < (A[i] -= B[i]);
        }
        for (; i < na && carry0; i++)
            carry0 = A[i] < (A[i] -= carry0);
        while (A.size() && !A.back())
            A.pop_back();
    }
    return r;
}

BigInteger BigInteger::operator*(const BigInteger &b) const
{
    BigInteger result;
    const size_t na = this->words.size(), nb = b.words.size();
    if (!nb || !na)
        return result;
    if ((na <= nb ? na : nb) > 20)
    {
        const std::vector<word> &A = this->words, &B = b.words;
        const size_t m2 = (na >= nb) ? (na / 2 + (na & 1)) : (nb / 2 + (nb & 1));
        BigInteger a0, a1, b0, b1;
        const bool hA = na > m2, hB = nb > m2;
        if (hA)
        {
            auto a_split = std::next(A.cbegin(), m2);
            a0.words = std::vector<word>(A.cbegin(), a_split);
	          while (a0.words.size() && !a0.words.back())
     	          a0.words.pop_back();
            a1.words = std::vector<word>(a_split, A.cend());
        }
        else
            a0.words = A;
        if (hB)
        {
            auto b_split = std::next(B.cbegin(), m2);
            b0.words = std::vector<word>(B.cbegin(), b_split);
	          while (b0.words.size() && !b0.words.back())
                b0.words.pop_back();
            b1.words = std::vector<word>(b_split, B.cend());
        }
        else
            b0.words = B;
        const BigInteger z0 = a0 * b0;
        if (hA && hB)
        {
            const BigInteger z1 = a1 * b1;
            result = z1;
            result.words.insert(result.words.begin(), m2, 0);
            result += (a1 + a0) * (b0 + b1) - z1 - z0;
        }
        else
            result = (a1 + a0) * (b0 + b1) - z0;
        result.words.insert(result.words.begin(), m2, 0);
        result += z0;
    }
    else
    {
        const std::vector<word> A = this->words, B = b.words;
        std::vector<word> &rsw = result.words;
        rsw.resize(na + nb + 1);
        word carry[2], aw[2], bw[2]; // temporary value
        for (size_t ia = 0, ib; ia < na; ia++)
        {
            aw[0] = A[ia] & WORD_HALF_MASK;
            aw[1] = A[ia] >> WORD_HALF_BITS;
            for (ib = 0; ib < nb; ib++)
            {
                auto i = rsw.begin() + ia + ib;
                auto j = rsw.end();
                bw[0] = B[ib] & WORD_HALF_MASK;
                bw[1] = B[ib] >> WORD_HALF_BITS;
                carry[0] = A[ia] * B[ib];
                carry[1] = ((aw[0] * bw[0]) >> WORD_HALF_BITS) + aw[1] * bw[0];
                carry[0] = (*(i++) += carry[0]) < carry[0];
                carry[1] = (carry[1] >> WORD_HALF_BITS) + ((aw[0] * bw[1] + (carry[1] & WORD_HALF_MASK)) >> WORD_HALF_BITS) + aw[1] * bw[1];
                carry[0] = ((*i += carry[0]) < carry[0]) + ((*(i++) += carry[1]) < carry[1]);
                while (carry[0] && (i != j))
                    carry[0] = (*(i++) += carry[0]) < carry[0];
                if (carry[0])
                    rsw.push_back(carry[0]);
            }
        }
    }
    while (result.words.size() && !result.words.back())
        result.words.pop_back();
    result.neg = this->neg ^ b.neg;
    return result;
}

BigInteger BigInteger::operator/(const BigInteger &b) const
{
    BigInteger quotient; //compare function
    bool cmp = true;
    word carry0, carry1;
    std::vector<word> rem = this->words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            for (i = nb - 1; i--;)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        quotient.words.reserve(n_words + 1);
        do
        {
            //compare function
            cmp = true;
	    na = rem.size();
            nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                }
                while (rem.size() && !rem.back())
                    rem.pop_back();
                quotient.set_bit(n);
            }
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
        quotient.neg = this->neg ^ b.neg;
    }
    return quotient;
}

BigInteger BigInteger::operator%(const BigInteger &b) const
{
    BigInteger remainder(*this);
    //compare function
    bool cmp = true;
    word carry0 = 0, carry1;
    std::vector<word> &rem = remainder.words, div = b.words;
    size_t na = rem.size(), nb = div.size(), i;
    if (na != nb)
        cmp = na > nb;
    else
    {
        i = na;
        while (i--)
        {
            carry0 = rem[i];
            carry1 = div[i];
            if (carry0 != carry1)
            {
                cmp = carry0 > carry1;
                break;
            }
        }
    }
    if (cmp)
    {
        //shifting count
        size_t n = (na - nb) * WORD_BITS;
        for (carry0 = rem.back(); carry0; carry0 >>= 1)
            n++;
        for (carry1 = div.back(); carry1; carry1 >>= 1)
            n--;
        const size_t n_bits = n % WORD_BITS;
        if (n_bits)
        {
            const size_t l_shift = WORD_BITS - n_bits;
            carry0 = div.back();
            carry1 = carry0 >> l_shift;
            if (carry1)
                div.push_back(carry1);
            for (i = nb - 1; i--;)
            {
                carry1 = div[i];
                div[i + 1] = (carry0 << n_bits) | (carry1 >> l_shift);
                carry0 = carry1;
            }
            div[0] = carry0 << n_bits;
        }
        const size_t n_words = n / WORD_BITS;
        if (n_words)
            div.insert(div.begin(), n_words, 0);
        do
        {
            //compare function
            cmp = true;
	    na = rem.size();
            nb = div.size();
            if (na != nb)
                cmp = na > nb;
            else
            {
                i = na;
                while (i--)
                {
                    carry0 = rem[i];
                    carry1 = div[i];
                    if (carry0 != carry1)
                    {
                        cmp = carry0 > carry1;
                        break;
                    }
                }
            }
            if (cmp)
            {
                carry0 = 0;
                for (i = 0; i < nb; i++)
                {
                    carry0 = rem[i] < (rem[i] -= carry0);
                    carry0 += rem[i] < (rem[i] -= div[i]);
                }
                for (; i < na && carry0; i++)
                    carry0 = rem[i] < (rem[i] -= carry0);
                while (rem.size() && !rem.back())
                    rem.pop_back();
            }
            nb--;
            word hi, lo = div[0];
            for (i = 0; i < nb; i++)
            {
                hi = div[i + 1];
                div[i] = (hi << WORD_BITS_1) | (lo >> 1);
                lo = hi;
            }
            div.back() = lo >> 1;
            if (!div.back())
                div.pop_back();
        } while (n-- && rem.size());
    }
    return remainder;
}


BigInteger BigInteger::operator^(size_t exponent) const
{
    BigInteger p = *this, result = 1;
    for (; exponent; exponent >>= 1)
    {
        if (exponent & 1)
        {
            result *= p;
            exponent--;
        }
        p *= p;
    }
    return result;
}

BigInteger BigInteger::operator-() const
{
    BigInteger result;
    result.words = this->words;
    result.neg = !this->neg;
    return result;
}

BigInteger &BigInteger::operator>>=(size_t n_bits)
{
    if (!n_bits)
        return *this;
    size_t j = n_bits / WORD_BITS;
    if (j >= words.size())
    {
        this->words.clear();
        return *this;
    }
    if (j)
        this->words.erase(this->words.begin(), this->words.begin() + j);

    const size_t n_len = words.size();
    n_bits %= WORD_BITS;
    if (n_bits)
    {
        word hi, lo = this->words[0];
        const size_t r_shift = WORD_BITS - n_bits;
        for (size_t i = 0; i < (n_len - 1); i++)
        {
            hi = this->words[i + 1];
            this->words[i] = (hi << r_shift) | (lo >> n_bits);
            lo = hi;
        }
        this->words.back() = lo >> n_bits;
        while (this->words.size() && !this->words.back())
            this->words.pop_back();
    }
    return *this;
}

BigInteger &BigInteger::operator<<=(size_t n_bits)
{
    if (!n_bits)
        return *this;
    const size_t j = n_bits / WORD_BITS;
    n_bits %= WORD_BITS;
    if (n_bits)
    {
        const size_t l_shift = WORD_BITS - n_bits;
        size_t i = this->words.size() - 1;
        word hi = this->words.back(), lo = hi >> l_shift;
        if (lo)
            this->words.push_back(lo);
        while (i--)
        {
            lo = this->words[i];
            this->words[i + 1] = (hi << n_bits) | (lo >> l_shift);
            hi = lo;
        }
        this->words[0] = hi << n_bits;
    }
    if (j)
        this->words.insert(this->words.begin(), j, 0);
    return *this;
}

BigInteger BigInteger::operator>>(size_t n_bits) const { return BigInteger(*this) >>= n_bits; }
BigInteger BigInteger::operator<<(size_t n_bits) const { return BigInteger(*this) <<= n_bits; }

std::ostream &operator<<(std::ostream &out, const BigInteger &num)
{
    std::vector<char> text;
    std::vector<word> A = num.words;
    text.reserve(LOG2BITS * A.size());
    word remainder, current;
    while (A.size())
    {
        remainder = 0;
        for (auto cur = A.rbegin(); cur != A.rend(); cur++)
        {
            current = *cur;
            remainder <<= WORD_HALF_BITS;
            remainder |= current >> WORD_HALF_BITS;
            *cur = remainder / 10;
            remainder %= 10;
            remainder <<= WORD_HALF_BITS;
            remainder |= current & WORD_HALF_MASK;
            *cur <<= WORD_HALF_BITS;
            *cur |= remainder / 10;
            remainder %= 10;
        }
        text.push_back('0' + char(remainder));
        while (A.size() && !A.back())
            A.pop_back();
    }
    if (num.neg)
        out << "-";
    std::reverse(text.begin(), text.end());
    text.push_back('\0');
    out << text.data();
    return out;
}

void karatsuba_test()
{
    {
        BigInteger A = 1111111;
        BigInteger res1;
        res1.words = karatsuba(A.words, A.words);
        std::cout << (A*A) << std::endl;
        std::cout << res1 << std::endl;
    }
    std::cout << "then" << std::endl;
    {
        BigInteger A("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
        BigInteger res1;
        res1.words = karatsuba(A.words, A.words);
        std::cout << (A*A) << std::endl;
        std::cout << res1 << std::endl;
    }
    std::cout << "then" << std::endl;
    {
        BigInteger A;
        A.words.push_back(0);
        A.words.push_back(0);
        A.words.push_back(89974321);
        A.words.push_back(1921);
        //A.words.resize(pow(2, ceil(log(A.words.size())/log(2))), 0);
        BigInteger res1;
        res1.words = karatsuba(A.words, A.words);
        std::cout << A << std::endl;
        std::cout << (A*A) << std::endl;
        std::cout << res1 << std::endl;
    }
}

std::vector<word> karatsuba(const std::vector<word> &A, const std::vector<word> &B)
{
    std::vector<word> result;
    const size_t na = A.size(), nb = B.size();
    if (na == 1 && nb == 1)
    {
        word a_hi = A[0] >> WORD_HALF_BITS;
        word a_lo = A[0] & WORD_HALF_MASK;
        word b_hi = B[0] >> WORD_HALF_BITS;
        word b_lo = B[0] & WORD_HALF_MASK;
        result.push_back(A[0] * B[0]);
        word carry = ((a_lo * b_lo) >> WORD_HALF_BITS) + a_hi * b_lo;
        carry = (carry >> WORD_HALF_BITS) + ((a_lo * b_hi + (carry & WORD_HALF_MASK)) >> WORD_HALF_BITS) + a_hi * b_hi;
        result.push_back(carry);
    }
    else if(na && nb)
    {
        const size_t m2 = (na >= nb) ? (na / 2 + (na & 1)) : (nb / 2 + (nb & 1));
        std::vector<word> a0, a1, b0, b1;
        if (na > m2)
        {
            auto a_split = std::next(A.cbegin(), m2);
            a0 = std::vector<word>(A.cbegin(), a_split);
            a1 = std::vector<word>(a_split, A.cend());
        }
        else
            a0.words = A;
        if (nb > m2)
        {
            auto b_split = std::next(B.cbegin(), m2);
            b0 = std::vector<word>(B.cbegin(), b_split);
            b1 = std::vector<word>(b_split, B.cend());
        }
        else
            b0.words = B;
        const std::vector<word> z0 = karatsuba(a0, b0);
        const std::vector<word> z1 = karatsuba(a1, b1);
        result = z1;
        result.insert(result.begin(), m2, 0);
        //add a0
        word carry = 0;
        size_t i = 0;
        for(; i < a1.size(); i++)
        {
            carry = (a0[i] += carry) < carry;
            carry += (a0[i] += a1[i]) < a1[i];
        }
        for(; carry && i < a0.size(); i++)
            carry = (a0[i] += carry) < carry;
        if (carry)
            a0.push_back(carry);
        //add b0
        carry = 0;
        i = 0;
        for(; i < b1.size(); i++)
        {
            carry = (b0[i] += carry) < carry;
            carry += (b0[i] += b1[i]) < b1[i];
        }
        for(; carry && i < b0.size(); i++)
            carry = (b0[i] += carry) < carry;
        if (carry)
            b0.push_back(carry);
        std::vector<word> mid = karatsuba(a0, b0);
        //sub mid with z0
        carry = 0
        i = 0;
        for (; i < z0.size(); i++)
        {
            carry = mid[i] < (mid[i] -= carry);
            carry += mid[i] < (mid[i] -= z0[i]);
        }
        for (; i < mid.size(); i++)
            carry = mid[i] < (mid[i] -= carry);
        //sub mid with z1
        carry = 0
        i = 0;
        for (; i < z1.size(); i++)
        {
            carry = mid[i] < (mid[i] -= carry);
            carry += mid[i] < (mid[i] -= z1[i]);
        }
        for (; i < mid.size(); i++)
            carry = mid[i] < (mid[i] -= carry);
        //add mid to result
        carry = 0;
        i = 0;
        for(; i < mid.size(); i++)
        {
            carry = (result[i] += carry) < carry;
            carry += (result[i] += mid[i]) < mid[i];
        }
        for(; carry && i < result.size(); i++)
            carry = (result[i] += carry) < carry;
        if (carry)
            result.push_back(carry);
        result.insert(result.words.begin(), m2, 0);
        //add z0 to result
        carry = 0;
        i = 0;
        for(; i < mid.size(); i++)
        {
            carry = (result[i] += carry) < carry;
            carry += (result[i] += z0[i]) < z0[i];
        }
        for(; carry && i < result.size(); i++)
            carry = (result[i] += carry) < carry;
        if (carry)
            result.push_back(carry);
    }
    return result;
}
