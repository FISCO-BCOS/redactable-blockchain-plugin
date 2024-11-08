use serde::{Deserialize, Serialize};

use crate::proxy::setup_msg::{ProxySetupPhaseBroadcastMsg,ProxySetupPhaseFinishFlag};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum GSTBKMsg {
    SetupMsg(SetupMsg),
    KeyGenMsg(KeyGenMsg),
}
#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum SetupMsg 
{
    ProxySetupPhaseBroadcastMsg(ProxySetupPhaseBroadcastMsg), 
    ProxySetupPhaseFinishFlag(ProxySetupPhaseFinishFlag)     
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum KeyGenMSKFlag {
    GammaA(KeyGenMsg),
    GammaB(KeyGenMsg),
    GammaO(KeyGenMsg),
    GammaC(KeyGenMsg)
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum KeyGenMsg {
    // ProxyToNodeKeyGenPhaseThreeP2PMsg(ProxyToNodeKeyGenPhaseThreeP2PMsg)
}