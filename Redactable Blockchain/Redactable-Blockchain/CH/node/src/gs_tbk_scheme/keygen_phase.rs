use std::fmt::format;
use std::fs::File;
use std::io::{BufRead, BufReader, Write};
use curv::arithmetic::traits::*;
use curv::elliptic::curves::{Secp256k1, Point, Scalar};
pub type FE = Scalar<Secp256k1>;
use curv::BigInt;
use log::info;
use num::traits::ops::bytes;
use sha2::{Digest, Sha256};
use crate::Error::{self};
use message::node::keygen_msg::NodeKeyGenPhaseOneBroadcastMsg;
use crate::node::Node;

static HEX_TABLE :[char;16] = ['0','1','2','3','4','5','6','7','8','9',
'A','B','C','D','E','F'];

impl Node { 
    pub fn keygen_phase_one(&mut self) -> NodeKeyGenPhaseOneBroadcastMsg
    {
        info!("Adapt is starting!");
        let key_path = "../../keypair/keypair".to_string() + &self.id.unwrap().to_string() + ".txt";
        let key_file = File::open(key_path).unwrap();
        let key_reader = BufReader::new(key_file);
        let mut key_lines = key_reader.lines().map(|l| l.unwrap());

        let random_path = "../../random/keypair".to_string() + &self.id.unwrap().to_string() + ".txt";
        let random_file = File::open(random_path).unwrap();
        let random_reader = BufReader::new(random_file);
        let mut random_lines = random_reader.lines().map(|l| l.unwrap());

        let ch_path = "src/node/ch.txt";
        let ch_file = File::open(ch_path).unwrap();
        let ch_reader = BufReader::new(ch_file);
        let mut ch_lines = ch_reader.lines().map(|l| l.unwrap());

        let block_path = "../../old_block.txt";
        let block_file = File::open(block_path).unwrap();
        let block_reader = BufReader::new(block_file);
        let mut block_lines = block_reader.lines().map(|l| l.unwrap());

        self.pk = serde_json::from_str(&key_lines.next().unwrap()).unwrap();
        let K:Point<Secp256k1> = serde_json::from_str(&random_lines.next().unwrap()).unwrap();

        let sk = FE::from_bigint(&BigInt::from_hex(&key_lines.next().unwrap()).unwrap());
        info!("sk is {:?}", sk);
        let k = FE::from_bigint(&BigInt::from_hex(&random_lines.next().unwrap()).unwrap());
        

        let H:Point<Secp256k1> = serde_json::from_str(&ch_lines.next().unwrap()).unwrap();

        let old_block = block_lines.next().unwrap();
        // let r_s_pk = &old_block[old_block.len() -  392..];
        // let r_str = Self::hex_to_string(&r_s_pk[..132]);
        // let s_str = Self::hex_to_string(&r_s_pk[132..260]);
        // let R = Point::<Secp256k1>::from_bytes(&Self::hex_string_to_bytes(&r_str)).unwrap();
        // let s = Scalar::<Secp256k1>::from_bytes(&Self::hex_string_to_bytes(&s_str));
        // info!("old_block {:?}", old_block);

        // let new_message = "Hello, World newset!";
        let new_message = "Hello World! (redacted)";
        let new_block = Self::update_message(&old_block, &new_message);
        self.new_block = new_block.clone();

        let index = new_block.find("060a0033").unwrap();
        let end_index = index + "060a0033".len();
        let new_hash_message = &new_block[..end_index];                           

        self.r_new = H + K;
        let message = new_hash_message.to_string() + &Self::to_hex(self.r_new.to_bytes(true).as_ref());
        let hash_str = Self::sha256(&message.as_str());
        let hash_new = FE::from_bigint(&BigInt::from_hex(&hash_str).unwrap());
        let s_new = k - hash_new.clone() * sk;
        self.h_new = hash_new;

        NodeKeyGenPhaseOneBroadcastMsg
        {
            sender:self.id.unwrap(),
            role:self.role.clone(),
            s_new:s_new.to_bigint()
        }
    }

    pub fn keygen_phase_two(&mut self, msg_vec:&Vec<NodeKeyGenPhaseOneBroadcastMsg>)
    -> Result<(), Error>
    {
        assert_eq!(msg_vec.len(), self.threashold_param.share_counts as usize);

        let key_path = "../../pk_share/pk_share".to_string() + &self.id.unwrap().to_string() + ".txt";
        let key_file = File::open(key_path.clone()).unwrap();
        let key_reader = BufReader::new(key_file);
        let mut key_lines = key_reader.lines().map(|l| l.unwrap());
        let key_commitment_vec: Vec<Point<Secp256k1>> = serde_json::from_str(&key_lines.next().unwrap()).unwrap();
        
        let random_path = "../../random_pk_share/pk_share".to_string() + &self.id.unwrap().to_string() + ".txt";
        let random_file = File::open(random_path).unwrap();
        let random_reader = BufReader::new(random_file);
        let mut random_lines = random_reader.lines().map(|l| l.unwrap());
        let random_commitment_vec: Vec<Point<Secp256k1>> = serde_json::from_str(&random_lines.next().unwrap()).unwrap();
        let mut flag = true;

        for msg in msg_vec{
            let s_new_g = random_commitment_vec[msg.sender as usize - 1].clone() - self.h_new.clone() * key_commitment_vec[msg.sender as usize -1].clone();
            let s_new_g_ = FE::from_bigint(&msg.s_new) * Point::generator();
            if s_new_g != s_new_g_ {flag = flag && false;} 
        }

        if flag {
            let mut lagrange_vec = Vec::new();
            for i in 0 ..= self.threashold_param.threshold as usize
            {
                lagrange_vec.push(BigInt::from(msg_vec.get(i).unwrap().sender));
            }
            
            let mut s_new_total = FE::zero();
            for i in 0 ..= self.threashold_param.threshold as usize
            {
                let msg = msg_vec.get(i).unwrap();
                let li = Self::map_share_to_new_params(BigInt::from(msg.sender), &lagrange_vec);
                s_new_total = s_new_total + FE::from_bigint(&msg.s_new) * li.clone();
    
            }
            info!("s_new_tatal is {:?}", s_new_total);
    
            let r = self.r_new.clone();
            let r_str = Self::bytes_to_hex_string(r.to_bytes(true).as_ref());
            let s_str = Self::bytes_to_hex_string(s_new_total.to_bytes().as_ref());
            let new_block = Self::update_rs(&self.new_block, &r_str, &s_str);
   
            let path = "../../new_block.txt";
            let file = File::create(path);
            match file.unwrap().write_all(new_block.as_bytes()){
                Ok(_) => println!("ch write in new_block.txt"),
                Err(err) => eprintln!("write new_block error: {}", err),
            }

            info!("CH adapt is finished");
            Ok
            (
                ()
            ) 
        }else {
            Err(Error::InvalidCom)
        }

        
    }


    pub fn to_hex(data : impl AsRef<[u8]>) -> String {
        let data = data.as_ref();
        let len = data.len();
        let mut res = String::with_capacity(len * 2);
    
        for i in 0..len {
        res.push(HEX_TABLE[usize::from(data[i] >> 4)] );
        res.push(HEX_TABLE[usize::from(data[i] & 0x0F)]);
        }
        res
        }

    pub fn hex_to_string(hex_str: &str) -> String {
        let bytes = hex::decode(hex_str).expect("Decoding failed");
        let result = std::str::from_utf8(&bytes).expect("Conversion to string failed");
        result.to_string()
    }

    pub fn string_to_hex(input_str: &str) -> String {
        let hex_str: String = input_str.bytes()
            .map(|byte| format!("{:02X}", byte))
            .collect();
        hex_str
    }

    pub fn hex_string_to_bytes(hex_str: &str) -> Vec<u8> {
        let mut bytes = Vec::new();
        let mut i = 0;
        while i < hex_str.len(){
            let byte = u8::from_str_radix(&hex_str[i..i+2], 16).unwrap();
            bytes.push(byte);
            i += 2;
        }
        bytes
    }

    pub fn bytes_to_hex_string(bytes: &[u8]) -> String {
        let hex_str:String = bytes.iter()
            .map(|byte| format!("{:02X}", byte))
            .collect();
        hex_str
    }

    pub fn sha256(input: &str) -> String {
        let mut hasher = Sha256::new();
        hasher.update(input.as_bytes());
        let result = hasher.finalize();
        // 将哈希值转换为16进制字符串
        result.to_vec().iter()
        .map(|b| format!("{:02x}", b))
        .collect::<String>()
    }

    pub fn update_rs(original_str: &str, r:&str, s:&str) -> String{
        let update_str = format!(
            "{}{}{}{}",
            &original_str[..original_str.len() - 392],
            Self::string_to_hex(r),
            Self::string_to_hex(s),
            &original_str[original_str.len() - 132..]
        );
        update_str
    }

    // pub fn update_message(original_str: &str, new_msg: &str) -> String{
    //     let new_msg_hex: String = new_msg.as_bytes().iter().map(|b| format!("{:02x}", b)).collect();
    //     //let old_msg_length = "Hello, World!".len();
    //     let old_msg_length = 32;
    //     let new_msg_length = 1062 - old_msg_length + new_msg.len();
    //     let new_msg_length_hex = format!("{:04x}", new_msg_length);
    
    //     let old_length_index = original_str.find("0426").unwrap();
    //     let old_msg_index = original_str.find("48656c6c6f2c20576f726c6421").unwrap();
    //     //let old_msg_hex = "48656c6c6f2c20576f726c6421";
    //     //let old_msg_hex_length = old_msg_hex.len();
    //     let old_msg_hex_length = 64;
    
    //     let update_str = format!(
    //         "{}{}{}{}{}",
    //         &original_str[..old_length_index],
    //         new_msg_length_hex,
    //         &original_str[old_length_index + 4..old_msg_index],
    //         new_msg_hex,
    //         &original_str[old_msg_index + old_msg_hex_length..]
    //     );
    
    //     update_str
    // }

    pub fn update_message(original_str: &str, new_msg: &str) -> String{
        let new_msg_hex: String = new_msg.as_bytes().iter().map(|b| format!("{:02x}", b)).collect();
        let padded_new_msg_hex = format!("{:0<64}",new_msg_hex);

        let old_msg_index = original_str.find("6020017f").unwrap() + 8;
        
        let update_str = format!(
            "{}{}{}",
            &original_str[..old_msg_index],
            padded_new_msg_hex,
            &original_str[(old_msg_index + 64)..]
        );

        update_str
    }

    pub fn map_share_to_new_params(index: BigInt, s: &[BigInt]) -> FE {
        let s_len = s.len();
        // add one to indices to get points
        let points: Vec<FE> = s.iter().map(|i| Scalar::from(i)).collect();
    
        let xi: FE = Scalar::from(&index);
        let num: FE = Scalar::from(&BigInt::from(1));
        let denum: FE = Scalar::from(&BigInt::from(1));
        let num = (0..s_len).fold(
            num,
            |acc, i| {
                if s[i] != index {
                    acc * &points[i]
                } else {
                    acc
                }
            },
        );
        let denum = (0..s_len).fold(denum, |acc, i| {
            if s[i] != index {
                let xj_sub_xi = &points[i] - &xi;
                acc * xj_sub_xi
            } else {
                acc
            }
        });
        let denum = denum.invert().unwrap();
        num * denum
    }
}

