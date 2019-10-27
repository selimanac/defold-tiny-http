
All requests and responses are JSON. You may consider of using [CJSON](https://github.com/Melsoft-Games/defold-cjson) for encoding and decoding.

## Examples

### Server

```lua
-- Server settings
local host = "localhost"
local port = 8800

-- Server callbacks
local function server_callbacks(self, message)
	-- Decode server message
    local jresult = json.decode(message.result)

    if jresult.server_status == dhttp.SERVER_START then
        -- Server started
    elseif jresult.server_status == dhttp.SERVER_STOP then
        -- Server stopped
    else
        pprint(jresult)
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
    -- Decode server message
    local jresult = json.decode(message.result)

	-- Check eroor
    if jresult.error then
        print("Error: ", jresult.error)
	end
	
	pprint(jresult)
end

function init(self)
    -- Init the client
    dhttp.client_start(host, port, client_callbacks)

    -- Get endpoint
    dhttp.client_get("/hi", 1)
end
```


## Server API

#### `dhttp.server_start(host, port, callback, [log], [error], [endpoints])`


Inits and starts the server.

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

#### Endpoints

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

#### `dhttp.client_get(endpoint, event_id)`

| Param  | Desc |
| ------------- | ------------- |
| `endpoint` | (_string_) Endpoint address  |
| `event_id`| (_int_) Event ID for tracking the action  |

Event IDs are for tracking the request on server and client. They will send as a header. You can easily group you triggers by using event ids.

#### `dhttp.client_post(endpoint, params, event_id)`

| Param  | Desc |
| ------------- | ------------- |
| `endpoint` | (_string_) Endpoint address  |
| `params` | (_string_) Json formated string  |
| `event_id`| (_int_) Event ID for tracking the action  |

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