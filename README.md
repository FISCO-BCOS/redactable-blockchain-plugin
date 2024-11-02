# 可编辑区块链部署文档



## 功能模块配置

###  可编辑区块链模块配置

1. 模块文件部署

   1. 可编辑区块链文件夹为模块文件夹，其下的Redactable-Blockchain部分每台机器都需要。

   2.  模块文件夹下对应节点文件夹下的intergration_test文件夹放到a中CH文件夹中。

2. 安装rust语言环境

   1. 首先确保安装有curl工具，可以通过apt命令安装：

      `sudo apt install curl`

   2. 通过下列命令下载安装脚本：

      `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`

   3. 启用rust，并测试是否可用

      `source $HOME/.cargo/env`

      `rustc --version`

3. 安装执行依赖

   1. 通过apt安装：gcc，g++，make，libgmp-dev，pari-gp，bison，libclang-dev。

      `sudo apt install gcc`

      `sudo apt install g++`

      `sudo apt install make`

      `sudo apt install libgmp-dev`

      `sudo apt install pari-gp`

      `sudo apt install bison`

      `sudo apt install libclang-dev`

4. 相互通信，包括一个proxy节点和四个node节点分别位于test10和test6-9上，文件路径为"/fisco/Redactable-Blockchain/CH/intergration_test"，**后续流程**中直接执行目录下的start_communication.sh进行节点的相互通信。

   `bash start_communication.sh`

   注：执行前修改位于"./src/proxy(node)/config/config_file"中的json文件，将其中的**ip地址进行相应的更新**，如下图：（参考）

   ![image-20241016154559842](.\pic\image-20241016154559842.png)

​				![image-20241016154617607](.\pic\image-20241016154617607.png)

### mysql以及data_export工具配置

1. mysql：

   1. 安装mysql服务器 

      `sudo apt install -y mysql-server`

   2. 启动mysql服务并设置自启动

      `sudo systemctl start mysql`

      `sudo systemctl enable mysql`

   3. 修改密码以及登录权限

      登录mysql：`sudo mysql;`

      切换到mysql库：`use mysql;``select user, host, plugin from user;`

      修改root用户的`auth_socket`权限，指定密码为**123456**

      `alter user 'root'@'localhost' identified with mysql_native_password by '123456';`

      应用修改：`flush privileges;`

   4. 测试sql服务：

      `mysql -uroot -p -h localhost -P 3306`

      ![image-20241014202749155](.\pic\image-20241014202749155.png)

      类似上图则说明配置成功。

   5. 创建data_export库用于存储导出的数据

      `create database data_export;`

   6. 执行Data-Export后，执行下述命令可以在sql中查看导出是否成功

      `use data_export;`

      `select count(*) from block_detail_info;`

      

2. data_export配置：

   ​	参考：[服务方式启动(对应链3.x版本)](https://data-doc.readthedocs.io/zh-cn/data_export_3.0/docs/WeBankBlockchain-Data-Export/install_Service_2.html)

   1. 拉取项目代码并切换版本：

      `git clone https://github.com/WeBankBlockchain/Data-Export.git`
      `cd Data-Export`
      `git checkout V3`

   2. 修改application.properties文件，位于"./tools/config"下，指定为sql服务的登录账号和密码。

      ![](.\pic\image-20241014205305431.png)

   3. 切换到节点文件夹，复制节点文件夹下的证书到tools/config下（路径信息记得更新！）

      `cp  ca/*  /home/test/fisco/Data-Export/tools/config`

      

### 数据导出工具storageTool生成

1. 该工具需要在编译源码时设置-DTOOLS=on编译选项，工具需要在节点目录下运行以读取节点的配置文件。即

   `cmake -DTOOLS=on ..`

   参考：**FISCO BCOS 联盟链国产化适配及部署文档**

   

### 敏感词检测模块配置

1. 敏感词检测部分放在fisco文件夹下，命名为sensitive_words_detection。

2. 配置连接mysql的参数

   ![image-20241015174308196](.\pic\image-20241015174308196.png)

3. 配置用到的敏感词词库

   ![image-20241015174431523](.\pic\image-20241015174431523.png)

4. 安装执行依赖

   `sudo apt install python3`

   `pip install pymysql`

   

## 前置准备流程

1. 搭建fisco-bcos链，参考：[搭建第一个区块链网络](https://fisco-bcos-doc.readthedocs.io/zh-cn/latest/docs/quick_start/air_installation.html)

   将新建节点计算验证哈希的Keccak256k1算法替换为变色龙hash，修改代码段位于FISCO-BCOS/bcos-tars-protocol/bcos-tars-protocol/protocol/TransactionFactoryImpl.h，**将修改后的代码文件替换原文件**。

2. 参考节点源码编译文档[ ：节点源码编译 — FISCO BCOS 3.0 v3.6.0 文档 (fisco-bcos-doc.readthedocs.io) ](https://fisco-bcos-doc.readthedocs.io/zh-cn/latest/docs/tutorial/compile_binary.html)

   注意：获取3.4.0分支代码：`git clone --branch v3.4.0 https://github.com/FISCO-BCOS/FISCO-BCOS.git`

3. 编译完成的二进制文件位于build/fisco-bcos-air，复制上面编译新生成二进制文件替换链文件中nodes/127.0.0.1文件夹下的二进制文件。

4. 使用CPP-SDK部署hello_world，参考：[CPP SDK — FISCO BCOS 3.0 v3.6.0 文档 (fisco-bcos-doc.readthedocs.io) ](https://fisco-bcos-doc.readthedocs.io/zh-cn/latest/docs/sdk/cpp_sdk/index.html)
   为了匹配节点底层修改后计算验签的变色龙哈希逻辑，同步修改CPP-SDK源码，替换以下五个文件，分别位于：

   sample/tx/deploy_hello.cpp、

   bcos-cpp-sdk/utilities/tx/Transaction.h、

   bcos-cpp-sdk/utilities/tx/TransactionBuilder.h、

   bcos-cpp-sdk/utilities/tx/TransactionBuilder.cpp、

   bcos-cpp-sdk/utilities/tx/TransactionBuilderInterface.h。    

   注：若要使input字段中待检测内容包含敏感词可以在deploy_hello.cpp文件中的 const char* hwBIN = ""；内容中找到hello，world的十六进制编码并替换为敏感词的编码后编译上链

5. 在CPP-SDK根目录下创建build文件夹，并编译对应的deploy_hello二进制文件。

   `mkdir build & cd build`

   `cmake ../ -DBUILD_SAMPLE=ON `

   `make`

   编译的deploy_hello二进制文件位于'build/sample/tx'中，将该二进制文件置于链文件中nodes/127.0.0.1文件夹下，部署该文件需要将clog.ini和config_sample.ini放到同级目录。

6. 配置证书

   `cp -r sdk/* ca`

   至此前置准备工作已完成。
## 区块编辑流程

1. 公私钥对生成，通过DKG生成对应节点的公私钥对，命名为keypair*.txt(位于Redactable-Blockchain/keypair中)，其中第一行为公钥，第二行为节点私钥。复制对应的公钥信息。

2. 区块发布，在对应的节点文件夹下执行

   `./deploy_hello ./config_sample.ini group0 复制过来的公钥`

3. 执行敏感词检测，定位到对应区块

   1. 确保完成[mysql以及data_export配置](###data_export配置：)，并启动mysql服务

   2. 进入Data_Export文件夹下的tools文件夹，启动工具，等待下载和初始化完成。

      `bash start.sh`

      如下图则说明全部同步成功

      ![image-20241015165415144](.\pic\image-20241015165415144.png)

   3. 切换到/home/test/fisco/sensitive_words_detection下，执行sensitive_words_detection.py（确保已完成[敏感词检测配置]()）。

      `python3 sensitive_words_detection.py`

      检测结果如下：（参考）

      ![image-20241015175133940](.\pic\image-20241015175133940.png)

4. 区块编辑

   1. 确保完成[数据导出工具storageTool生成]()，将该工具放到node0文件夹下。

   2. 在fisco console查看存在敏感词的区块信息(以第二个区块为例)

      `getBlockByNumber 2`

      查询结果如下图：（参考）

      ![image-20241015175456716](.\pic\image-20241015175456716.png)

      注：交易hash指的是transactions中的hash

      `0xf342e643848785828d2e479927bd099867f97cd68a6ef12174e908dc1fef2925`

   3. 切换到node0文件夹下，使用storageTool读取敏感词所在区块的具体信息。

      `./storageTool -r s_hash_2_tx ***(交易hash) -H 1`

      查询结果如下：(参考)

      ![image-20241015180344304](.\pic\image-20241015180344304.png)

   4.  将区块信息(value=***)复制到Redactable-Blockchain/old_block.txt中(没有就创建对应的文件)

   5. 确保完成[CH配置]()，切换到CH/intergration_test文件夹下，执行对应的脚本，等待new_block.txt的生成(和old_block.txt处于同级目录)。
      `bash start_communication.sh`

   6. 停止所有节点，切换到fisco/(ip子文件夹)下，执行

      `bash stop_all.sh`

   7. 修改所有区块的信息，复制new_block.txt中的新的区块信息，执行

      `./storageTool -w s_hash_2_tx ***(交易hash) ***(new_block_value) -H 1`

      注：命令已集成write_new_block.sh，执行只需传入交易hash参数，将该脚本放到node0文件夹下。

      注：四个节点都需要执行该命令进行修改

   8. 查看修改是否成功，在fisco console执行

      `getBlockByNumber X`

      或者通过

      `./storageTool -r s_hash_2_tx ***(交易hash) -H 1`

   9. 区块编辑完成
