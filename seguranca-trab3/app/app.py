"""APLICAÇÃO PROVISÓRIA (Membro A) - só para o Módulo 1 ter um login para capturar.

O Membro C substitui por uma versão com senha em hash, HMAC etc., mantendo o contrato:
  GET  /                -> página com formulário de login
  POST /login           -> campos "usuario" e "senha" (form urlencoded)
                           ok: 303 + Set-Cookie: sessao=...; HttpOnly   | erro: 401
  GET  /area-protegida  -> 200 com cookie de sessão válido | 401 sem sessão
  GET  /saude           -> 200 "ok"
"""
import os
import secrets
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs

USUARIOS = {"alice": "SenhaSecreta123"}  # provisório: o Membro C guarda em hash
SESSOES = {}                             # token -> usuário
COOKIE_SECURE = os.environ.get("CENARIO") == "protegido"

FORMULARIO = """<!doctype html><html><body><h1>Intranet do laboratório</h1>
<form method="post" action="/login">
 Usuário <input name="usuario"> Senha <input name="senha" type="password">
 <button>Entrar</button></form></body></html>"""


class Handler(BaseHTTPRequestHandler):
    def responder(self, codigo, corpo, cabecalhos=None):
        dados = corpo.encode()
        self.send_response(codigo)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(dados)))
        for nome, valor in (cabecalhos or {}).items():
            self.send_header(nome, valor)
        self.end_headers()
        self.wfile.write(dados)

    def usuario_logado(self):
        cookie = SimpleCookie(self.headers.get("Cookie", ""))
        token = cookie["sessao"].value if "sessao" in cookie else None
        return SESSOES.get(token)

    def do_GET(self):
        if self.path == "/":
            self.responder(200, FORMULARIO)
        elif self.path == "/saude":
            self.responder(200, "ok")
        elif self.path == "/area-protegida":
            usuario = self.usuario_logado()
            if usuario:
                self.responder(200, f"<h1>Bem-vinda, {usuario}!</h1><p>Dados internos.</p>")
            else:
                self.responder(401, "<h1>401 - faça login</h1>")
        else:
            self.responder(404, "<h1>404</h1>")

    def do_POST(self):
        if self.path != "/login":
            return self.responder(404, "<h1>404</h1>")
        tamanho = int(self.headers.get("Content-Length", 0))
        campos = parse_qs(self.rfile.read(tamanho).decode())
        usuario = campos.get("usuario", [""])[0]
        senha = campos.get("senha", [""])[0]
        if USUARIOS.get(usuario) == senha:
            token = secrets.token_hex(16)
            SESSOES[token] = usuario
            atributos = "Path=/; HttpOnly" + ("; Secure" if COOKIE_SECURE else "")
            self.responder(303, "", {"Location": "/area-protegida",
                                     "Set-Cookie": f"sessao={token}; {atributos}"})
        else:
            self.responder(401, "<h1>401 - usuário ou senha inválidos</h1>")


if __name__ == "__main__":
    print(f"[app] ouvindo em 0.0.0.0:8000 (cookie Secure={COOKIE_SECURE})")
    ThreadingHTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
