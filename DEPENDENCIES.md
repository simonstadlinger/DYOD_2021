# Dependencies

| Name             | Version       | Platform |                Optional |
| ---------------- | ------------- | -------- | ----------------------- |
| build-essential  | any           |    Linux |                      No |
| boost            | >= 1.63.0     |    All   |                      No |
| clang            | >= 9.0        |    All   |   Yes, if gcc installed |
| clang-format     | >= 9.0        |    All   |        Yes (formatting) |
| cmake            | >= 3.5        |    All   |                      No |
| gcc              | >= 9.1        |    All   | Yes, if clang installed |
| gcovr            | >= 3.2        |    All   |          Yes (coverage) |
| parallel         | any           |    All   |                     Yes |
| python           | 3             |    All   |           Yes (linting) |


## Dependencies that are integrated in our build process via git submodules
- googletest (https://github.com/google/googletest)
