#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <cmath>

// if you do not plan to implement bonus, you can delete those lines
// or just keep them as is and do not define the macro to 1
#define SUPPORT_IFSTREAM 0
#define SUPPORT_ISQRT 0
#define SUPPORT_EVAL 0 // special bonus

#define MAXDIGITS 9

class BigInteger
{
public:
    // constructors
    BigInteger() { numbers.push_back(0u); }

    BigInteger(int64_t n) {
        if (n == 0)
        {
            numbers.push_back(0u);
            return;
        }
        
        
        if (n < 0)
        {
            isNegative = true;
        }


        while (n) {
            int64_t num = n % 1'000'000'000;
            num = abs(num);
            numbers.push_back(num);

            n /= 1'000'000'000;
        }

        fixZero(*this);
    }

    explicit BigInteger(const std::string& str) {

        if (str.empty())
            throw std::runtime_error("empty string");
        

        if (std::isspace(str.front()) || std::isspace(str.back()))
            throw std::runtime_error("white space in string");
        

        bool hasSign = false;

        if (str[0] == '-') {
            isNegative = true;
            hasSign = true;
        }
        else if (str[0] == '+') {
            hasSign = true;
        }



        for (int64_t i = str.size(); i >= 0; i -= MAXDIGITS) {

            std::string temp;

            if (i <= MAXDIGITS) {
                temp = str.substr(0, i);
                
                if (hasSign) {
                    temp.erase(temp.begin());
                }
            }
            else {
                temp = str.substr(i - MAXDIGITS, MAXDIGITS);

            }

            if (!temp.empty())
                numbers.push_back((std::stoull(temp)));
        }


        //remove leading 0
        while (numbers.back() == 0 && numbers.size() > 1) {
            numbers.pop_back();
        }

        fixZero(*this);
    }


    // copy
    BigInteger(const BigInteger& other) = default;
    BigInteger& operator=(const BigInteger& rhs) = default;


    // unary operators
    const BigInteger& operator+() const {
        return *this;
    }
    
    BigInteger operator-() const {
        BigInteger copy(*this);
        
        copy.isNegative = !isNegative;

        fixZero(copy);
        return copy;
    }


    // binary arithmetics operators
    BigInteger& operator+=(const BigInteger& rhs) {
        if (isNegative != rhs.isNegative)
            return *this -= -rhs;



        numbers.resize(std::max(numbers.size(), rhs.numbers.size()), 0);


        uint64_t carry = 0;

        for (size_t i = 0; i != numbers.size(); i++)
        {
            if (i < rhs.numbers.size())
                carry += rhs.numbers[i];


            numbers[i] += carry;
            carry = numbers[i] / 1000000000;
            
            numbers[i] %= 1000000000;

        }


        if (carry)
            numbers.push_back(carry);

        
        fixZero(*this);
        return *this;
    }

    BigInteger& operator-=(const BigInteger& rhs) {
        if (isNegative != rhs.isNegative)
            return *this += -rhs;

        if ((!isNegative && *this < rhs) || (isNegative && *this > rhs)) {
            *this = rhs - *this;
            isNegative = !isNegative;

            fixZero(*this);
            return *this;
        }


        int64_t borrow = 0;

        for (size_t i = 0; i < numbers.size(); ++i) {
            int64_t diff = numbers[i] - borrow;
            if (i < rhs.numbers.size()) {
                diff -= rhs.numbers[i];
            }

            if (diff < 0) {
                diff += 1000000000;
                borrow = 1;
            }
            else {
                borrow = 0;
            }

            numbers[i] = diff;
        }

        // Resize to remove leading zeros
        while (numbers.size() > 1 && numbers.back() == 0) {
            numbers.pop_back();
        }

        fixZero(*this);
        return *this;
    }

    BigInteger& operator*=(const BigInteger& rhs) { 
        isNegative = isNegative != rhs.isNegative;
        
        std::vector<uint64_t> result(numbers.size() + rhs.numbers.size(), 0);

        for (size_t i = 0; i < numbers.size(); ++i) {
            uint64_t carry = 0;

            for (size_t j = 0; j < rhs.numbers.size() || carry; ++j) {
                // Calculate the current product
                uint64_t current = result[i + j] + (numbers[i]) * (j < rhs.numbers.size() ? rhs.numbers[j] : 0) + carry;

                // Update the current digit with the current product and handle carry
                result[i + j] = current % 1000000000;
                carry = current / 1000000000;
            }
        }

        while (result.size() > 1 && result.back() == 0) {
            result.pop_back();
        }

        numbers = result;
        
        fixZero(*this);
        return *this;
    }

    BigInteger& operator/=(const BigInteger& rhs) 
    {   
        if (isZero(rhs))
            throw std::runtime_error("Division by zero");
        
        if (isZero(*this))
            return *this;

        if (rhs.numbers.size() == 1 && rhs.numbers[0] == 1)
            return *this;


        bool quotientIsNegative = this->isNegative != rhs.isNegative;
        
        BigInteger divisor = rhs;

        this->isNegative = false;
        divisor.isNegative = false;
       
        BigInteger quotient{}; //0
        int64_t tempQuotient = 0;
        
        
        while (*this >= divisor) {
            uint64_t shift = 0;
        
            while (*this >= (divisor << shift)) {
                ++shift;
            }
            
            --shift;

            while (*this >= (divisor << shift))
            {
                *this -= (divisor << shift);
                tempQuotient += 1;
            }

            quotient += (BigInteger(tempQuotient) << shift);
            tempQuotient = 0;
        }

        *this = quotient;
        this->isNegative = quotientIsNegative;

        fixZero(*this);
        return *this; 
    }

    //teoreticky sa tu nemusi spravit divisor (kopia rhs) ani tie zmeny isNegative, ak by sme chceli iba kladne cisla
    BigInteger& operator%=(const BigInteger& rhs) 
    {
        if (isZero(rhs))
            throw std::runtime_error("Modulo by zero");

        if (isZero(*this))
            return *this;

        if (rhs.numbers.size() == 1 && rhs.numbers[0] == 1) {
            *this = 0;

            return *this;
        }


        BigInteger divisor = rhs;

        bool remainderIsNegative = this->isNegative != rhs.isNegative;

        this->isNegative = false;
        divisor.isNegative = false;


        while (*this >= divisor) {
            uint64_t shift = 0;

            while (*this >= (divisor << shift)) {
                ++shift;
            }

            --shift;

            while (*this >= (divisor << shift))
            {
                *this -= (divisor << shift);
            }
        }

        this->isNegative = remainderIsNegative;

        fixZero(*this);
        return *this;
    }


    double sqrt() const {
        if (isNegative)
            throw std::runtime_error("Cannot calculate SQRT of negative number");
        
        return std::sqrt(convertToDouble(*this));
    }

#if SUPPORT_ISQRT
    BigInteger isqrt() const;
#endif

private:
    bool isNegative = false;
    std::vector<uint64_t> numbers;

    friend inline BigInteger operator+(BigInteger lhs, const BigInteger& rhs);
    friend inline BigInteger operator-(BigInteger lhs, const BigInteger& rhs);
    friend inline BigInteger operator*(BigInteger lhs, const BigInteger& rhs);
    friend inline BigInteger operator/(BigInteger lhs, const BigInteger& rhs);
    friend inline BigInteger operator%(BigInteger lhs, const BigInteger& rhs);

    friend inline bool operator==(const BigInteger& lhs, const BigInteger& rhs);
    friend inline bool operator!=(const BigInteger& lhs, const BigInteger& rhs);
    friend inline bool operator<(const BigInteger& lhs, const BigInteger& rhs);
    friend inline bool operator>(const BigInteger& lhs, const BigInteger& rhs);
    friend inline bool operator<=(const BigInteger& lhs, const BigInteger& rhs);
    friend inline bool operator>=(const BigInteger& lhs, const BigInteger& rhs);

    friend inline std::ostream& operator<<(std::ostream& lhs, const BigInteger& rhs);

    friend inline BigInteger operator<<(BigInteger lhs, uint64_t shift);

    friend bool isZero(const BigInteger& num);
    friend void fixZero(BigInteger& num);

    friend bool getNegativeSign(const BigInteger& num);
    friend void setNegativeSign(BigInteger& num, bool isNegative);

    friend double convertToDouble(const BigInteger& num);
};

inline BigInteger operator+(BigInteger lhs, const BigInteger& rhs) { return lhs += rhs; }
inline BigInteger operator-(BigInteger lhs, const BigInteger& rhs) { return lhs -= rhs; }
inline BigInteger operator*(BigInteger lhs, const BigInteger& rhs) { return lhs *= rhs; }
inline BigInteger operator/(BigInteger lhs, const BigInteger& rhs) { return lhs /= rhs; }
inline BigInteger operator%(BigInteger lhs, const BigInteger& rhs) { return lhs %= rhs; }

inline bool operator==(const BigInteger& lhs, const BigInteger& rhs) { return (lhs.isNegative == rhs.isNegative) && (lhs.numbers == rhs.numbers); }
inline bool operator!=(const BigInteger& lhs, const BigInteger& rhs) { return !(lhs == rhs); }
inline bool operator<(const BigInteger& lhs, const BigInteger& rhs) {
    if (lhs.isNegative != rhs.isNegative)
        return lhs.isNegative;

    if (lhs.numbers.size() != rhs.numbers.size())
        return (lhs.numbers.size() < rhs.numbers.size()) != lhs.isNegative;

    for (std::size_t i = lhs.numbers.size(); i-- > 0;) {
        if (lhs.numbers[i] != rhs.numbers[i])
            return (lhs.numbers[i] < rhs.numbers[i]) != lhs.isNegative;
    }

    return false;
}
inline bool operator>(const BigInteger& lhs, const BigInteger& rhs) { return rhs < lhs; }
inline bool operator<=(const BigInteger& lhs, const BigInteger& rhs) { return !(lhs > rhs); }
inline bool operator>=(const BigInteger& lhs, const BigInteger& rhs) { return !(lhs < rhs); }

inline std::ostream& operator<<(std::ostream& os, const BigInteger& rhs) {
    if (rhs.isNegative)
        os << '-';


    os << rhs.numbers.back();
    for (int i = rhs.numbers.size() - 2; i >= 0; --i) {
        os << std::setfill('0') << std::setw(9) << rhs.numbers[i];
    }

    return os;
}

inline BigInteger operator<<(BigInteger lhs, uint64_t shift) {
    if (shift == 0)
        return lhs;

    std::vector<uint64_t> trailingZerosVec;
    while (shift >= 9)
    {
        trailingZerosVec.push_back(0u);
        shift -= 9;
    }

    int64_t trailingZeros = static_cast<int64_t>(pow(10, shift));

    lhs *= trailingZeros;


    lhs.numbers.insert(lhs.numbers.begin(), trailingZerosVec.begin(), trailingZerosVec.end());


    return lhs;
}

bool isZero(const BigInteger& num) {
    return (num.numbers.size() == 1 && num.numbers[0] == 0);
}

void fixZero(BigInteger& num) {
    if (isZero(num))
        num.isNegative = false;
}

bool getNegativeSign(const BigInteger& num) {
    return num.isNegative;
}

void setNegativeSign(BigInteger& num, bool isNegative) {
    num.isNegative = isNegative;
}

double convertToDouble(const BigInteger& num) {
    double convertedNum = 0;
    const double maxDouble = std::numeric_limits<double>::max();


    for (auto it = num.numbers.rbegin(); it != num.numbers.rend(); ++it)
    {
        convertedNum *= 1000000000.0;

        if (convertedNum > maxDouble - *it)
            throw std::runtime_error("Double overflow");

        convertedNum += *it;
    }

    return convertedNum;
}



#if SUPPORT_IFSTREAM == 1
// this should behave exactly the same as reading int with respect to 
// whitespace, consumed characters etc...
inline std::istream& operator>>(std::istream& lhs, BigInteger& rhs); // bonus
#endif


class BigRational
{
public:
    // constructors
    BigRational() {}
    
    BigRational(int64_t a, int64_t b) {
        numerator = a;
        denominator = b;

        isNegative = getNegativeSign(numerator) != getNegativeSign(denominator);

        setNegativeSign(numerator, false);
        setNegativeSign(denominator, false);

        simplify(*this);
    }
    
    BigRational(const std::string& a, const std::string& b) {
        numerator = BigInteger(a);
        denominator = BigInteger(b);

        isNegative = getNegativeSign(numerator) != getNegativeSign(denominator);

        setNegativeSign(numerator, false);
        setNegativeSign(denominator, false);

        simplify(*this);
    }

    // copy
    BigRational(const BigRational& other) = default;
    BigRational& operator=(const BigRational& rhs) = default;
    
    // unary operators
    const BigRational& operator+() const { return *this; }

    BigRational operator-() const {
        BigRational copy(*this);

        copy.isNegative = !isNegative;

        simplify(copy);
        return copy;

    }
    
    // binary arithmetics operators
    BigRational& operator+=(const BigRational& rhs) {
        numerator = (numerator * rhs.denominator) + (rhs.numerator * denominator);
        denominator *= rhs.denominator;

        simplify(*this);
        return *this;
    }

    BigRational& operator-=(const BigRational& rhs) {
        numerator = (numerator * rhs.denominator) - (rhs.numerator * denominator);
        denominator *= rhs.denominator;

        if (getNegativeSign(numerator))
        {
            isNegative = !isNegative;
            setNegativeSign(numerator, false);
        }

        simplify(*this);
        return *this;
    }

    BigRational& operator*=(const BigRational& rhs) {
        isNegative = (isNegative != rhs.isNegative);
        
        numerator *= rhs.numerator;
        denominator *= rhs.denominator;

        simplify(*this);
        return *this;
    }

    BigRational& operator/=(const BigRational& rhs) {
        isNegative = (isNegative != rhs.isNegative);

        numerator *= rhs.denominator;
        denominator *= rhs.numerator;

        simplify(*this);
        return *this;
    }

    double sqrt() const {
        if (isNegative)
            throw std::runtime_error("Cannot calculate SQRT of negative number");

        return std::sqrt((convertToDouble(numerator) / convertToDouble(denominator)));
    }
#if SUPPORT_ISQRT
    BigInteger isqrt() const;
#endif

private:
    BigInteger numerator{};
    BigInteger denominator{1};
    bool isNegative = false;


    friend inline BigRational operator+(BigRational lhs, const BigRational& rhs);
    friend inline BigRational operator-(BigRational lhs, const BigRational& rhs);
    friend inline BigRational operator*(BigRational lhs, const BigRational& rhs);
    friend inline BigRational operator/(BigRational lhs, const BigRational& rhs);

    friend inline bool operator==(const BigRational& lhs, const BigRational& rhs);
    friend inline bool operator!=(const BigRational& lhs, const BigRational& rhs);
    friend inline bool operator<(const BigRational& lhs, const BigRational& rhs);
    friend inline bool operator>(const BigRational& lhs, const BigRational& rhs);
    friend inline bool operator<=(const BigRational& lhs, const BigRational& rhs);
    friend inline bool operator>=(const BigRational& lhs, const BigRational& rhs);

    friend inline std::ostream& operator<<(std::ostream& lhs, const BigRational& rhs);

    friend void simplify(BigRational& fraction);
};

inline BigRational operator+(BigRational lhs, const BigRational& rhs) { return lhs += rhs; }
inline BigRational operator-(BigRational lhs, const BigRational& rhs) { return lhs -= rhs; }
inline BigRational operator*(BigRational lhs, const BigRational& rhs) { return lhs *= rhs; }
inline BigRational operator/(BigRational lhs, const BigRational& rhs) { return lhs /= rhs; }

inline bool operator==(const BigRational& lhs, const BigRational& rhs) {
    if (lhs.isNegative == rhs.isNegative)
    {
        if (lhs.denominator != rhs.denominator)
        {
            BigInteger a = lhs.numerator * rhs.denominator;
            BigInteger b = rhs.numerator * lhs.denominator;
            return a == b;
        }

        return lhs.numerator == rhs.numerator;
    }

    return false;
}
inline bool operator!=(const BigRational& lhs, const BigRational& rhs) { return !(lhs == rhs); }
inline bool operator<(const BigRational& lhs, const BigRational& rhs) {
    if (lhs.isNegative != rhs.isNegative)
        return lhs.isNegative;

    if (lhs.denominator != rhs.denominator)
    {
        BigInteger a = lhs.numerator * rhs.denominator;
        BigInteger b = rhs.numerator * lhs.denominator;
        return a < b;
    }

    return lhs.numerator < rhs.numerator;
}
inline bool operator>(const BigRational& lhs, const BigRational& rhs) { return rhs < lhs; }
inline bool operator<=(const BigRational& lhs, const BigRational& rhs) { return !(lhs > rhs); }
inline bool operator>=(const BigRational& lhs, const BigRational& rhs) { return !(lhs < rhs); }

inline std::ostream& operator<<(std::ostream& os, const BigRational& rhs) {
    if (rhs.isNegative)
        os << '-';

    os << rhs.numerator;

    if (rhs.denominator != 1)
    {
        os << '/' << rhs.denominator;
    }
    
    return os;
}

void simplify(BigRational& fraction) {
    if (isZero(fraction.numerator))
    {
        fraction.denominator = 1;
        fraction.isNegative = false;
    }
    
    BigInteger a = fraction.numerator;
    BigInteger b = fraction.denominator;
    

    while (!isZero(b))
    {
        BigInteger temp = b;
        b = a % b;
        a = temp;
    }

    fraction.numerator /= a;
    fraction.denominator /= a;
}


#if SUPPORT_IFSTREAM == 1
// this should behave exactly the same as reading int with respect to 
// whitespace, consumed characters etc...
inline std::istream& operator>>(std::istream& lhs, BigRational& rhs); // bonus
#endif

#if SUPPORT_EVAL == 1
inline BigInteger eval(const std::string&);
#endif