use serde::{Deserialize, Serialize};
use crate::node::setup_msg::{NodeToProxySetupPhaseP2PMsg, NodeSetupPhaseFinishFlag};
use crate::node::keygen_msg::NodeKeyGenPhaseOneBroadcastMsg;
#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum GSTBKMsg {
    SetupMsg(SetupMsg),
    KeyGenMsg(KeyGenMsg),
}
#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum SetupMsg 
{
    NodeToProxySetupPhaseP2PMsg(NodeToProxySetupPhaseP2PMsg),
    NodeSetupPhaseFinishFlag(NodeSetupPhaseFinishFlag)     
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub enum KeyGenMsg {
    NodeKeyGenPhaseOneBroadcastMsg(NodeKeyGenPhaseOneBroadcastMsg)
}
