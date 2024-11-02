#!/bin/bash
echo "start proxy now"
cargo 'test' '--package' 'intergration_test' '--lib' '--' 'proxy::proxy_node::test' '--exact' '--nocapture'
