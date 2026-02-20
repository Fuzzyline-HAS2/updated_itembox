import sys
import traceback
from tests.itembox_model import ItemBoxModel
from tests.fake_server import FakeServer
from tests.net_shim import NetShim
from tests import test_reset_recovery

def run_test_func(func, name):
    print(f"Running {name}...")
    model = ItemBoxModel()
    model.boot()
    server = FakeServer()
    server.bind(model.on_server_message, model.rfid_tag)
    
    try:
        func(model, server)
        print("PASS")
    except Exception:
        print("FAIL")
        traceback.print_exc()

if __name__ == "__main__":
    run_test_func(test_reset_recovery.test_reconnect_recovery, "test_reconnect_recovery")
