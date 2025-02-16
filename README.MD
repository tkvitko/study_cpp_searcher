# SEARCH Engine
## Installation
### Requires:
``Boost`` 
``libpqxx``

### Build:
```
mkdir build
cd build
cmake ../
cmake --build .
```

## Usage
### Config example:
```
[Db]
host=localhost
port=5432
name=searcher
userName=searcher
password=searcher
[Crowler]
startPageDomain=wiki.openssl.org
startPagePath=/
recursionDepth=2
[Searcher]
serverIp=0.0.0.0
serverPort=9093
```

## Running
### To collect data:
``./crowler``

### To run http-server to process search queries:
``./http_server``

Open http://127.0.0.1:9093/search
