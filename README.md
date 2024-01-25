# C++ header with types for arbitrary precision integers and fractions

`BigInteger` 
- type for arbitrary precision int
- represented as `std::vector` of 9 digit long `uint64_t`

`BigRational`
- type for arbitrary precision fractions
- represented as 2 `BigInteger` (numerator and denominator)

Supported operators
1. unary `+ -`
2. binary `+ - * / %(only for BigInteger)`
3. relational `== != < > <= >=`
4. `sqrt()` (with lost precision)
5. `isqrt()` (floor sqrt without loosing precision)
6. output stream `<<`
