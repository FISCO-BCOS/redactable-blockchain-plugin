
use std::{fmt::format, fs::File, io::{BufRead, BufReader, Write}};

use curv::{arithmetic::Converter, elliptic::curves::{Point, Scalar, Secp256k1}, BigInt};
use sha2::{digest::Update, Digest, Sha256};

pub type CU = Secp256k1;
pub type FE = Scalar<Secp256k1>;
pub type GE = Point<Secp256k1>;

static HEX_TABLE :[char;16] = ['0','1','2','3','4','5','6','7','8','9',
                                        'A','B','C','D','E','F'];

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

pub fn sha256(input: &str) -> String {
    let mut hasher = Sha256::new();
    Digest::update(&mut hasher, input.as_bytes());
    let result = hasher.finalize();
    // 将哈希值转换为16进制字符串
    result.to_vec().iter()
        .map(|b| format!("{:02x}", b))
        .collect::<String>()
}

pub fn generate_hash(pk:GE, R:GE, s:FE, message:String) -> GE {
    let G = Point::generator();
    let hash_str = sha256(&(message + &to_hex(R.to_bytes(true).as_ref())));
    let hash = FE::from_bigint(&BigInt::from_hex(&hash_str).unwrap());
    let H = R - (hash * pk + s * G);
    return H;
}

pub fn adapt(sk:FE, new_message:String, hash:GE) -> (GE, FE){
    let G = Point::generator();
    let k = FE::random();
    let R =  hash + k.clone() * G;
    let message = new_message + &to_hex(R.to_bytes(true).as_ref());
    let hash_str = sha256(&message.as_str());
    let hash_new = FE::from_bigint(&BigInt::from_hex(&hash_str).unwrap());
    let s = k - hash_new * sk;
    return (R, s)
}

pub fn verify(pk:GE, R:GE, s:FE, message:String, H:GE) -> bool{
    let G = Point::generator();
    let hash_str = sha256(&(message + &to_hex(R.to_bytes(true).as_ref())));
    let hash = FE::from_bigint(&BigInt::from_hex(&hash_str).unwrap());
    let H_ = R - (hash * pk + s * G);
    return H == H_;
}

#[test]
fn get_hash(){
    let path = "src/node/node1/keypair.txt";
    let file = File::open(path).unwrap();
    let reader = BufReader::new(file);
    let mut lines = reader.lines().map(|l| l.unwrap());
    let pk:Point<Secp256k1> = serde_json::from_str(&lines.next().unwrap()).unwrap();
    let G = Point::generator();
    let r = FE::random();
    let R = r * G;
    let s = FE::random();
    let H = generate_hash(pk, R.clone(), s.clone(), "12345".to_string());
    println!("H is {:?}", H );
    let ch_str = serde_json::to_string(&H).unwrap() + "\n" + &serde_json::to_string(&R).unwrap() + "\n" + &s.to_bigint().to_hex();

    let path = "src/node/ch.txt";
    let file = File::create(path);
    match file.unwrap().write_all(ch_str.as_bytes()){
        Ok(_) => println!("ch write in ch.txt"),
        Err(err) => eprintln!("write ch error: {}", err),
    }
    // generate_hash(pk, R, s, message)
}

fn update_message(original_str: &str, new_msg: &str) -> String{
    let new_msg_hex: String = new_msg.as_bytes().iter().map(|b| format!("{:02x}", b)).collect();
    let old_msg_length = "Hello, World!".len();
    let new_msg_length = 1062 - old_msg_length + new_msg.len();
    let new_msg_length_hex = format!("{:04x}", new_msg_length);

    let old_length_index = original_str.find("0426").unwrap();
    let old_msg_index = original_str.find("48656c6c6f2c20576f726c6421").unwrap();
    let old_msg_hex = "48656c6c6f2c20576f726c6421";
    let old_msg_hex_length = old_msg_hex.len();

    let update_str = format!(
        "{}{}{}{}{}",
        &original_str[..old_length_index],
        new_msg_length_hex,
        &original_str[old_length_index + 4..old_msg_index],
        new_msg_hex,
        &original_str[old_msg_index + old_msg_hex_length..]
    );

    update_str
}

#[test]
fn test(){

    let original_str = "1a2606636861696e30360667726f7570304101f456273134393337333830313830363638313535393030323431363136343434363630383635363537387d00010426608060405234801561001057600080fd5b506040518060400160405280600d81526020017f48656c6c6f2c20576f726c6421000000000000000000000000000000000000008152506000908051906020019061005c929190610062565b50610107565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f106100a357805160ff19168380011785556100d1565b828001600101855582156100d1579182015b828111156100d05782518255916020019190600101906100b5565b5b5090506100de91906100e2565b5090565b61010491905b808211156101005760008160009055506001016100e8565b5090565b90565b610310806101166000396000f3fe608060405234801561001057600080fd5b50600436106100365760003560e01c80634ed3885e1461003b5780636d4ce63c146100f6575b600080fd5b6100f46004803603602081101561005157600080fd5b810190808035906020019064010000000081111561006e57600080fd5b82018360208201111561008057600080fd5b803590602001918460018302840111640100000000831117156100a257600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610179565b005b6100fe610193565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561013e578082015181840152602081019050610123565b50505050905090810190601f16801561016b5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b806000908051906020019061018f929190610235565b5050565b606060008054600181600116156101000203166002900480601f01602080910402602001604051908101604052809291908181526020018280546001816001161561010002031660029004801561022b5780601f106102005761010080835404028352916020019161022b565b820191906000526020600020905b81548152906001019060200180831161020e57829003601f168201915b5050505050905090565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f1061027657805160ff19168380011785556102a4565b828001600101855582156102a4579182015b828111156102a3578251825591602001919060010190610288565b5b5090506102b191906102b5565b5090565b6102d791905b808211156102d35760008160009055506001016102bb565b5090565b9056fea2646970667358221220b5943f43c48cc93c6d71cdcf27aee5072566c88755ce9186e32ce83b24e8dc6c64736f6c634300060a00330b2d000020c33caa27c183bb4acba7b02fb669354a9c3c47e436a068888ecadd0e8ba976173d000041064d18fda1c44cc5a897ac5e67c0cbf2337d94fe5fbe62f68379277732250b8e7627e80c2e531c2f2b1009e46aa6c66c41d5718006d55dccff8cf3e19361e883004300000190577e7bb47d0000141153e1e87f77fbeaf90df29fc7f05435c0c740fd86c430324244433346374435423837303742313138313730333143314539314138413843314442433942434143313337434543463939343236423530453944343341314632344135413734453143463538323532444241463746463143343632463830463946423546393835323533344241424145423537414146383744413139393234303236423446303832313933303441323632334531394135453932413232323730384344324330323131383041443638423538423444463646334138383645434441";
    let new_msg = "Hello newset";
    let new_message = update_message(original_str, new_msg);
    println!("{}", new_message);

    let G = Point::generator();
    let r = FE::random();
    let R = r * G;
    let s = FE::random();
    let sk = FE::random();
    let pk = GE::generator() * sk.clone();

    let hash = generate_hash(pk.clone(), R.clone(), s.clone(), "123".to_string());

    let (R_new, s_new) = adapt(sk.clone(), "1234".to_string(), hash.clone());
    
    let ret = verify(pk.clone(), R, s, "123".to_string(), hash.clone());
    println!("ret: {}", ret);

    let ret_ = verify(pk.clone(), R_new, s_new, "1234".to_string(), hash);
    println!("ret_: {}", ret_);
    
}
