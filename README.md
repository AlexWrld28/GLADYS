# GLADYS
Initial implementation of our geography oriented language

## Example

```gld
let x = 42;
let origin = point(0.0, 0.0);
let nyc = point(-73.9857, 40.7484);
let proximity = distance(origin, nyc);

print(nyc);
print(proximity);
```

Running this program prints:

```text
point(-73.9857, 40.7484)
84.4649
```

Points use the `point(x, y)` literal form. Both coordinates may be integers or
floating-point values, and each coordinate may be negative.

Proximity measurements use `distance(left, right)`, where each argument is an
expression that resolves to a point. The parser accepts both named points and
inline point literals:

```gld
print distance(origin, nyc);
print distance(point(0.0, 0.0), point(3.0, 4.0));
```
