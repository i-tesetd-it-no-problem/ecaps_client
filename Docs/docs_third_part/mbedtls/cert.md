# 证书配置
注: 可直接跳到 [快速生成](#4-快速生成) 章节。

## 目录
- [1. 创建CA及根证书](#1-创建ca及根证书)
- [2. 使用CA签发服务器证书](#2-使用ca签发服务器证书)
  - [2.1 创建服务端配置文件 `server.cnf`](#21-创建服务端配置文件-servercnf)
  - [2.2 生成服务器私钥和证书签名请求(CSR)](#22-生成服务器私钥和证书签名请求csr)
  - [2.3 使用CA签署服务器证书](#23-使用ca签署服务器证书)
- [3. 使用CA签发客户端证书](#3-使用ca签发客户端证书)
  - [3.1 创建客户端配置文件 `client.cnf`](#31-创建客户端配置文件-clientcnf)
  - [3.2 生成客户端私钥和证书签名请求(CSR)](#32-生成客户端私钥和证书签名请求csr)
  - [3.3 使用CA签署客户端证书](#33-使用ca签署客户端证书)
- [4. 快速生成](#4-快速生成)
  - [4.1 新建脚本文件`create_certificates.sh`](#41-新建脚本文件create_certificatessh)
  - [4.2 执行脚本](#42-执行脚本)
- [5. 复制客户端证书到指定平台](#5-复制客户端证书到指定平台)

## 1. 创建CA及根证书

```shell
cd ssl_cfg

# 生成 CA 私钥
openssl genrsa -out ca.key 4096

# 生成 CA 自签名根证书
openssl req -new -x509 -days 3650 -key ca.key -out ca.pem
```

在执行 命令时,系统会提示您输入一些信息：

- **Country Name (2 letter code) [XX]**: 输入国家代码,例如 CN 表示中国。
- **State or Province Name (full name) []**: 输入省份名称,例如 ShangHai。
- **Locality Name (eg, city) [Default City]**: 输入城市名称,例如 ShangHai。
- **Organization Name (eg, company) [Default Company Ltd]**: 输入您的组织或公司名称,例如 My Company。
- **Organizational Unit Name (eg, section) []**: 输入组织单位名称,例如 IT Department。
- **Common Name (eg, your name or your server's hostname) []**: 输入 CA 的名称,例如 My Root CA。
- **Email Address []**: 输入联系邮箱,或留空。

## 2. 使用 CA 签发服务器证书

### 2.1 创建服务端配置文件 `server.cnf`

在 `ssl_cfg` 目录下创建 `server.cnf` 文件,内容如下：

```ini
[ req ]
default_bits       = 2048
prompt             = no
default_md         = sha256
distinguished_name = dn
req_extensions     = v3_req

[ dn ]
C  = CN
ST = ShangHai
L  = ShangHai
O  = My Company
OU = IT Department
CN = your.domain.com  # 替换为您的服务器域名或 IP

[ v3_req ]
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = your.domain.com   # 如果使用域名,替换为您的域名
IP.1  = xxx.xxx.xxx.xxx    # 如果使用 IP,替换为您的服务器 IP 地址
```

### 2.2 生成服务器私钥和证书签名请求(CSR)

```shell
# 生成服务器私钥
openssl genrsa -out server.key 2048

# 生成服务器 CSR
openssl req -new -key server.key -out server.csr -config server.cnf
```

### 2.3 使用 CA 签署服务器证书

```shell
# 签署服务器证书
openssl x509 -req -in server.csr -CA ca.pem -CAkey ca.key \
  -CAcreateserial -out server.crt -days 365 -sha256 \
  -extensions v3_req -extfile server.cnf
```

## 3. 使用 CA 签发客户端证书

### 3.1 创建客户端配置文件 `client.cnf`

在 `ssl_cfg` 目录下创建 `client.cnf` 文件,内容如下：

```ini
[ req ]
default_bits       = 2048
prompt             = no
default_md         = sha256
distinguished_name = dn
req_extensions     = v3_req

[ dn ]
C  = CN
ST = ShangHai
L  = ShangHai
O  = My Company
OU = IT Department
CN = client   # 客户端的唯一标识符,例如设备名称

[ v3_req ]
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = client.local  # 替换为客户端的实际名称或 IP
```

### 3.2 生成客户端私钥和证书签名请求(CSR)

```shell
# 生成客户端私钥
openssl genrsa -out client.key 2048

# 生成客户端 CSR
openssl req -new -key client.key -out client.csr -config client.cnf
```

### 3.3 使用 CA 签署客户端证书

```shell
# 签署客户端证书
openssl x509 -req -in client.csr -CA ca.pem -CAkey ca.key \
  -CAcreateserial -out client.crt -days 365 -sha256 \
  -extensions v3_req -extfile client.cnf
```

## 4. 快速生成
### 4.1 新建脚本文件
进入到 `ssl_cfg` 目录下,新建脚本文件 `create_certificates.sh`,
内容如下：
```shell
#!/bin/bash

# 变量定义
ROOT_CA_NAME="My Root CA"     # 根CA名
COUNTRY="CN"                  # 国家
STATE="ShangHai"              # 州
CITY="ShangHai"               # 城市
ORGANIZATION="My Company"     # 公司名
ORG_UNIT="IT Department"      # 组织
SERVER_CN="your.domain.com"   # 替换为 服务器域名 或 IP
SERVER_IP="xxx.xxx.xxx.xxx"   # 替换为服务器 IP
CLIENT_CN="client"            # 客户端名
CLIENT_DNS="client.local"     # 客户端DNS

# 创建 ssl_cfg 目录并进入
mkdir -p ssl_cfg
cd ssl_cfg

# 生成 CA 私钥
openssl genrsa -out ca.key 4096

# 生成 CA 自签名根证书
openssl req -new -x509 -days 3650 -key ca.key -out ca.pem \
  -subj "/C=$COUNTRY/ST=$STATE/L=$CITY/O=$ORGANIZATION/OU=$ORG_UNIT/CN=$ROOT_CA_NAME"

# 创建服务器的 OpenSSL 配置文件 server.cnf
cat > server.cnf <<EOL
[ req ]
default_bits       = 2048
prompt             = no
default_md         = sha256
distinguished_name = dn
req_extensions     = v3_req

[ dn ]
C  = $COUNTRY
ST = $STATE
L  = $CITY
O  = $ORGANIZATION
OU = $ORG_UNIT
CN = $SERVER_CN

[ v3_req ]
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = $SERVER_CN
IP.1  = $SERVER_IP
EOL

# 生成服务器私钥
openssl genrsa -out server.key 2048

# 生成服务器 CSR
openssl req -new -key server.key -out server.csr -config server.cnf

# 使用 CA 签署服务器证书
openssl x509 -req -in server.csr -CA ca.pem -CAkey ca.key \
  -CAcreateserial -out server.crt -days 365 -sha256 \
  -extensions v3_req -extfile server.cnf

# 创建客户端的 OpenSSL 配置文件 client.cnf
cat > client.cnf <<EOL
[ req ]
default_bits       = 2048
prompt             = no
default_md         = sha256
distinguished_name = dn
req_extensions     = v3_req

[ dn ]
C  = $COUNTRY
ST = $STATE
L  = $CITY
O  = $ORGANIZATION
OU = $ORG_UNIT
CN = $CLIENT_CN

[ v3_req ]
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = $CLIENT_DNS
EOL

# 生成客户端私钥
openssl genrsa -out client.key 2048

# 生成客户端 CSR
openssl req -new -key client.key -out client.csr -config client.cnf

# 使用 CA 签署客户端证书
openssl x509 -req -in client.csr -CA ca.pem -CAkey ca.key \
  -CAcreateserial -out client.crt -days 365 -sha256 \
  -extensions v3_req -extfile client.cnf

echo "all certification generated successful"
```

### 4.2 执行脚本
```shell
chmod +x create_certificates.sh
./create_certificates.sh
```

## 5. 复制客户端证书到指定平台
客户端应用需要使用
- 根证书 `ca.pem`
- 客户端证书 `client.crt`
- 客户端私钥 `client.key`