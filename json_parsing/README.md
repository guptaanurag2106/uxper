# Implementing a JSON Serializer, Deserializer

## Context
- [JSON](https://www.json.org/json-en.html)

Big JSONs https://github.com/antonmedv/json-examples

## Build & Run

```bash
# Build
make

# Run
./main <json_file>
```

## Comparing with cJSON
### Time
```
$ ./main tests/data_250mb.json
json.h time taken: 768.383000ms
cJSON time taken: 1895.475000ms
$ ./main tests/data_10mb.json
json.h time taken: 45.491000ms
cJSON time taken: 89.329000ms
```

### Output
```
$ ./main tests/data_10mb.json > temp
$ wc temp ; tail -n 1 temp
  342526   954686 12701897 temp
cJSON time taken: 198.030000ms

$ ./main tests/data_10mb.json > temp
$ wc temp ; tail -n 1 temp
  342526   954686 14903578 temp
json.h time taken: 98.358000ms
```
 (The character difference could be due to space vs tabs for indenting)

