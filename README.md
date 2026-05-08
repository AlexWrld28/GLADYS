# GLADYS
Initial implementation of our geography oriented language

## Example

```gld
let x = 42;
let origin = point(0.0, 0.0);
let nyc = point(-73.9857, 40.7484);

print(nyc);
```

Points use the `point(x, y)` literal form. Both coordinates may be integers or
floating-point values, and each coordinate may be negative.
