#!/usr/bin/env python3
from json import *
import sys
import csv
import binascii
import pymysql
import csv
import argparse

if __name__ == '__main__':

    # 创建一个ArgumentParser对象
    parser = argparse.ArgumentParser(description="A python script to connect to a database")

    # 添加命令行参数
    parser.add_argument("--host", type=str, default='localhost', help="The host name or IP address of the database server(default:localhost)")
    parser.add_argument("--port", type=int, default=3306, help="The port number of the database server(default:3306)")
    parser.add_argument("--username", type=str, default='root', help="The user name for the database connection(default:root)")
    parser.add_argument("--password", type=str, default='123456', help="The password for the database connection")
    parser.add_argument("--database", type=str, default='data_export', help="The name of the database to use")
    parser.add_argument("--dictionary", type=str, default='sensitive_words.txt', help="The name of the dictionary to use")
    # # 解析命令行参数
    # args = parser.parse_args()

    # # 如果没有提供任何参数，打印使用说明并退出
    # if len(sys.argv) == 1:
    #     parser.print_help()
    #     sys.exit(1)

    # dictionary = args.dictionary
    dictionary = 'sensitive_words.txt'

    def hexStr_to_str(hex_str):
        hex = hex_str.encode('utf-8')
        str_bin = binascii.unhexlify(hex)
        return str_bin.decode('utf-8')


    class DFAUtils(object):
        """
        DFA算法：敏感词过滤
        """

        def __init__(self):
            """
            算法初始化
            """
            # 敏感词链表
            self.root = dict()
            # 敏感词限定
            self.delimit = '\x00'
            # 无意义词库,在检测中需要跳过的（这种无意义的词最后有个专门的地方维护，保存到数据库或者其他存储介质中）
            self.skip_root = [' ', '&', '!', '！', '@', '#', '$', '￥', '*', '^', '%', '?', '？', '<', '>', "《", '》', "，", "。",
                            ",", ".", "\\", "|", "/", "\'", "\"", "“", "‘", ";", "；", ":", "：", "~", "`", "-", "+", "=",
                            "_"]
            # 初始化敏感词库
            for sensitive_word in self.read_sensitive_words():
                self.add_word(sensitive_word)

        @staticmethod
        def read_sensitive_words():
            """
            读取敏感词库
            :return: 返回敏感词列表
            """
            sensitive_words_list = []
            with open(dictionary, mode='r', encoding='utf-8') as fp:
            #with open('sensitive_words.txt', mode='r', encoding='utf-8') as fp:
                sensitive_words = fp.readlines()
                for sensitive_word in sensitive_words:
                    # 敏感词英文变为小写、去除首尾空格和换行
                    sensitive_word = sensitive_word.lower().strip()
                    # 如果敏感词为空直接跳过
                    if not sensitive_word:
                        continue
                    # 将敏感词添加到敏感词列表
                    sensitive_words_list.append(sensitive_word)

            return sensitive_words_list

        def add_word(self, sensitive_word):
            """
            敏感词入库
            :param sensitive_word: 敏感词
            """
            now_node = self.root
            # 遍历敏感词的每个字
            for i in range(len(sensitive_word)):
                # 如果这个字已经存在字符链的key中就进入其子字典
                if sensitive_word[i] in now_node:
                    now_node = now_node[sensitive_word[i]]
                else:
                    if not isinstance(now_node, dict):
                        break
                    for j in range(i, len(sensitive_word)):
                        now_node[sensitive_word[j]] = {}
                        last_level, last_char = now_node, sensitive_word[j]
                        now_node = now_node[sensitive_word[j]]
                    last_level[last_char] = {self.delimit: 0}
                    break
            if i == len(sensitive_word) - 1:
                now_node[self.delimit] = 0

        def match_word(self, message, repl="", need_first=False, need_all=False):
            """
            匹配敏感词
            :param message: 待检测的文本
            :param repl: 用于替换的字符，匹配的敏感词以字符逐个替换，如"你是大王八"，敏感词"王八"，替换字符*，替换结果"你是大**"
            :param need_first: True，返回匹配到的第一个敏感词
            :param need_all: True，返回匹配到的全部敏感词
            :return: 根据 need_first、need_all、need_replace 返回对应的结果
            """
            message = message.lower()
            ret = []
            start = 0
            sensitive_word = []
            while start < len(message):
                level = self.root
                # 是否违禁
                is_unlawful = False
                checked_chars = []
                for char in message[start:]:
                    start += 1
                    if not char.strip():
                        continue
                    if char in self.skip_root and (need_first or need_all):
                        continue
                    checked_chars.append(char)
                    # 判断连续是否为敏感词，及敏感词的长度
                    if char in level:
                        # 循环结束的条件，条件中有限定符：
                        # 1. 只有一个元素
                        # 2. 有多个元素，且其中有一个限定符，且下一个元素不在条件中
                        # 3. 文本结束，且有限定符，没有下一个字符
                        # 4. 不满足条件，则不需要替换
                        if self.delimit in level[char]:

                            if len(level[char]) == 1 or (start < len(message) and message[start] not in level[char]) or start == len(message):
                                is_unlawful = True
                                sensitive_word.append(''.join(checked_chars))
                                if need_first:
                                    return sensitive_word
                                break

                        level = level[char]
                    elif char in self.skip_root:
                        continue
                    else:
                        break

                if is_unlawful:
                    append_str = self.replace_char(checked_chars, repl)
                    ret.append(append_str)
                else:
                    if len(checked_chars) > 1:
                        start = start - len(checked_chars) + 1
                        ret.append(checked_chars[0])
                    else:
                        ret.extend(checked_chars)

            if need_all and sensitive_word:
                sensitive_word = sorted(set(sensitive_word), key=sensitive_word.index)

            return ''.join(ret) if not (need_first or need_all) else sensitive_word

        @staticmethod
        def replace_char(checked_chars, repl):
            """
            :param checked_chars: 被检测字符
            :param repl: 用于替换的字符，匹配的敏感词以字符逐个替换
            :return: 根据 repl 返回替换的字符
            """
            replace_msg = len(checked_chars) * repl if repl else checked_chars
            return replace_msg

        def get_first_word(self, message):
            """
            获取匹配到的词语
            :param message: 待检测的文本
            :return: 返回匹配到的第一个敏感词
            """
            first_word = self.match_word(message, need_first=True)
            return first_word[0] if first_word else "文本不涉及敏感词汇"

        def get_all_word(self, message):
            """
            获取匹配到的词语
            :param message: 待检测的文本
            :return: 返回匹配到的全部敏感词
            """
            all_word = self.match_word(message, need_all=True)
            return all_word if all_word else "文本不涉及敏感词汇"

        def replace_match_word(self, message):
            """

            获取匹配到的词语
            :param message: 待检测的文本
            :return: 返回已替换匹配到的全部敏感词的文本
            """
            replace_message = self.match_word(message, repl="*")
            return replace_message

    dfa_sensitive_words = DFAUtils()

    conn = pymysql.connect(
        
        # host=args.host,
        # port=args.port,
        # user=args.username,
        # password=args.password,
        # database=args.database,
        # charset='utf8'
        host="127.0.0.1",
        port=3306,
        user="root",
        password="123456",
        database="data_export",
        charset='utf8'
        
    )
    
    # 创建游标
    cursor = conn.cursor()
    # sql语句书写
    sql = "select * from data_export.tx_receipt_raw_data"
    # 执行sql
    cursor.execute(sql)
    # 将sql语句查询到的内容全都保存到data变量中
    data = cursor.fetchall()
    #print(data)
    # 关闭数据库连接
    cursor.close()

    header = ('pk_id','block_hash','block_height','block_time_stamp','contract_address','depot_updatetime','from','gas_used','input','logs','logs_bloom','message','output','receipt_proof','root','status','to','tx_hash','tx_index','tx_proof')
    with open ("data_export.tx_receipt_raw_data.csv", "w+", newline = "") as file:
        pen = csv.writer(file)
        pen.writerow(header)
        # 遍历data中的每一行，并且转换为列表格式 （默认 tuple）
        for i in data:
            a = list(i)
            pen.writerow(a)

# 提取区块中input，即交易数据部分
    my_columns = ['input']
    my_columns_index = []
    with open('data_export.tx_receipt_raw_data.csv', 'r', newline='') as csv_in_file:
        with open('input_data.csv', 'w', newline='') as  csv_out_file:
            filereader = csv.reader(csv_in_file)
            filewriter = csv.writer(csv_out_file)
            header = next(filereader,None)
            for index_value in range(len(header)):
                if header[index_value] in my_columns:
                    my_columns_index.append(index_value)
            for row_list in filereader:
                row_list_output = []
                for index_value in my_columns_index:
                    row_list_output.append(row_list[index_value])
                filewriter.writerow(row_list_output)
    
    #将十六进制的数据转换为字符串类型输出，并对每一行数据进行敏感词检测，排除注册信息，定位到第几个区块存在敏感词汇
    #print(sys._getframe().f_lineno)
    i = 0
    with open("input_data.csv", "r", encoding='utf-8') as fp:
        with open("output_str_data.csv", "r+", encoding='utf-8') as f:
            msg = fp.readline()
            while msg:
                result = 'ffffffffffffffffffffffffffffffffffffffff' in msg
                if (result):
                    msg = fp.readline()
                    i = i + 1
                # elif (`transfer`):
                else:
                    i = i + 1

                    line = msg[76:140] #获取数据部分
                    print(line)
                    line1 = str(bytes.fromhex(line),'utf-8')
                    line1 = line1.replace('\x00','')
                    result=dfa_sensitive_words.get_all_word(line1)
                    if result != '文本不涉及敏感词汇':
                        print('第', i, '个区块交易数据存在敏感词汇:', dfa_sensitive_words.get_all_word(line1))
                    f.writelines(line1 + '\n')
                    msg = fp.readline()
        f.close()
    fp.close()
