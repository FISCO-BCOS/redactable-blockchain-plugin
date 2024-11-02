#!/bin/bash
echo "start node1 now"
sleep 2
cargo 'test' '--package' 'intergration_test' '--lib' '--' 'node::node1::node1::test' '--exact' '--nocapture'









