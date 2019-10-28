![Tiny Http Native Extension](https://selimanac.github.io/assets/gfx/tiny_http_dark.png)

Tiny Http Native Extension is a simple http server and client for the Defold Game Engine. 

All requests and responses are JSON. You may consider of using [CJSON](https://github.com/Melsoft-Games/defold-cjson) for encoding and decoding.

All server responses are static echo of the request. POST responses are limited but customizable by using [`dhttp.server_post_content`](#dhttpserver_post_contentjson).

## Installation
You can use Tiny Http in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/defold-tiny-http/archive/master.zip
	
---

## Examples

Detailed server and client examples can be found in [examples](https://github.com/selimanac/defold-tiny-http/tree/master/examples) folder.

### Server

```lua
-- Server settings
local host = "localhost"
local port = 8800

-- Server callbacks
local function server_callbacks(self, message)
	-- Decode response
    local jresult = json.decode(message.result)
    local event_id = message.event_id

    if jresult.server_status == dhttp.SERVER_START then
        -- Server started
    elseif jresult.server_status == dhttp.SERVER_STOP then
        -- Server stopped
    else
    	if event_id == 123 then
        	pprint(jresult)
       end
    end
end

function init(self)
    -- Start the server
    dhttp.server_start(host, port, server_callbacks)
end
```

### Client
```lua
-- Client settings
local host = "localhost"
local port = 8800

-- Client callbacks
local function client_callbacks(self, message)
    -- Decode response
    local jresult = json.decode(message.result)

    -- Check error
    if jresult.error then
        print("Error: ", jresult.error)
    end

    pprint(jresult)
end

function init(self)
    -- Init the client
    dhttp.client_start(host, port, client_callbacks)

    -- Get endpoint
    dhttp.client_get("/hi", 123)
end
```


## Server API

#### `dhttp.server_start(host, port, callback, [log], [error], [endpoints])`


Init and start the server.

| Param  | Desc |
| ------------- | ------------- |
| `host`  | (_string_) Host address or IP  |
| `port`  | (_int_) Port number  |
| `callback`  | (_function_) Callback function  |
| `[log]`  | (_boolean_) Turn logging on/off. Default is false |
| `[error]`  | (_boolean_) Turn error responses on/off. Default is true |
| `[endpoints]`  | (_table_) Optional endpoints |

#### `dhttp.server_stop()`

Gracefully shutdown the server.

#### `dhttp.is_server_running()`

Check if server is running. Returns boolean.

#### `dhttp.server_post_content([json])`

Sets the POST response for all POST endpoints. 

### Endpoints

There are several built-in endpoints.

| Endpoint  | Desc |
| ------------- | ------------- |
| "/hi" | Says hi!  |
| "/num/(\d+)" | (_GET_) Gets only [0-9] as integer  |
| "/str/(\w+)" | (_GET_) Gets only [a-zA-Z0-9] as string  |
| "/post" | (_POST_) Generic post endpoint with params  |
| "/stop" | (_GET_) Stop the server  |

You can define custom endpoints.  
Endpoints support regex. But "?" character is reserved and may cause a problem.


```lua
local endpoints = {
    {
        endpoint_type = dhttp.METHOD_GET,
        endpoint = "/monster/(\\d+)"
    },
    {
        endpoint_type = dhttp.METHOD_POST,
        endpoint = "/move"
    }
}

dhttp.server_start("localhost", 8888, server_callbacks, false, true, endpoints)

```

## Client API

#### `dhttp.client_start(host, port, client_callbacks)`

Init the client.

#### `dhttp.client_hi()`

Defold says hi!

#### `dhttp.client_get(endpoint, [event_id])`

| Param  | Desc |
| ------------- | ------------- |
| `endpoint` | (_string_) Endpoint address  |
| `[event_id]`| (_int_) Event ID for tracking the action  |

Event IDs are for tracking the requests on server and client. They send as a header. You can easily group and parse you triggers by using event ids.

#### `dhttp.client_post(endpoint, params, [event_id])`

| Param  | Desc |
| ------------- | ------------- |
| `endpoint` | (_string_) Endpoint address  |
| `params` | (_string_) Json formated string  |
| `[event_id]`| (_int_) Event ID for tracking the action  |

```lua
local temp_table = {
        x = 10,
        y = 20
    }

    jresult = cjson.encode(temp_table)

    local params = {
        position = jresult
    }

    dhttp.client_post("/post", params, 1)
```


#### Constants

##### dhttp.METHOD_GET
##### dhttp.METHOD_POST
##### dhttp.SERVER_START
##### dhttp.SERVER_STOP

## Dependencies

Build by using [cpp-httplib](https://github.com/yhirose/cpp-httplib)
