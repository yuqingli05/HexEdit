# HexEdit

#### 介绍
一个针对 hex 和 bin 文件进行 合并 删除 裁剪 清零 的命令行工具。mcu开发过程中 可以方便的 bin转hex hex转bin。对 hex和bin 合并，分离等操作

#### 使用方式
~~~
.\HexEdit.exe --help
        --add 执行文件合并操作,把输入的文件合并成一个文件,当没有输入命令时候add是默认命令
        --del 对输入文件删除指定地址和长度 地址:长度 eg: --sub 0x00000000:1024 (十六进制:十进制)
        --cut 对输入文件裁剪出指定地址和长度 eg: --cut 0x00000000:1024 (十六进制:十进制)
        --set 对输入文件指定地址进行充填操作 地址:长度:充填字节 eg: --set 0x00000000:1024:0xFF(十六进制:十进制:十六进制)
        -f 后面跟输入的文件名，最多输入64个文件
        -o 后面跟输出的文件名
        -b 指定后面输入和输出的文件类型为二进制文件,直到再次遇见改变文件类型的参数
        -h 指定后面输入和输出的文件类型为hex文件,直到再次遇见改变文件类型的参数
        -x 指定后面输入二进制文件的开始地址,如果不指定的话直接在后面拼接, eg:-x 0x00
        -i 设置输出二进制文件的充填字节,设置-1将分段输出  eg:-i 255(设置充填字节为0xFF,默认0xFF)
        --version 输出版本信息
        --help    输出帮助信息
~~~


