# C++ header with datatypes for arbitrary precision integers and fractions

### `BigInteger` 
- type for arbitrary precision int
- represented as `std::vector` of 9 digit long `uint64_t`

### `BigRational`
- type for arbitrary precision fractions
- represented as 2 `BigInteger` (numerator and denominator)

Both datatypes behave the same as other datatypes representing numbers such as `int` or `double`

---

## Supported operators
1. unary `+ -`
2. binary `+ - * / %(only for BigInteger)`
3. relational `== != < > <= >=`
4. `sqrt()` (with lost precision)
5. `isqrt()` (floor sqrt without loosing precision)
6. output stream `<<`
