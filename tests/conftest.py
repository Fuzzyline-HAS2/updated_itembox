"""
Pytest Configuration & Fixtures
===============================
테스트 실행 시 필요한 공통 객체(Model, Server)를 생성하고 연결.
"""

import pytest
from tests.itembox_model import ItemBoxModel, State
from tests.fake_server import FakeServer
from tests.net_shim import NetShim

@pytest.fixture
def model():
    """깨끗한 상태의 ItemBox 모델"""
    m = ItemBoxModel()
    m.boot()
    return m

@pytest.fixture
def server(model):
    """모델과 연결된 Fake Server"""
    srv = FakeServer()
    # 서버 -> 모델 연결 (직접 호출)
    srv.bind(model.on_server_message, model.rfid_tag)
    return srv

@pytest.fixture
def shim_server(model):
    """
    모델과 연결되지만 중간에 네트워트 장애(NetShim)가 끼어있는 서버.
    (네트워크 테스트 전용)
    """
    srv = FakeServer()
    # NetShim을 통해 연결
    shim_msg = NetShim(model.on_server_message)
    shim_rfid = NetShim(model.rfid_tag)
    
    # 서버는 Shim의 send를 호출 -> Shim이 처리 후 -> 모델 호출
    srv.bind(shim_msg.send, shim_rfid.send)
    
    return srv, shim_msg, shim_rfid
