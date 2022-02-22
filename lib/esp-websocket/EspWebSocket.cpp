#include <ArduinoJson.h>
#include "EspWebSocket.h"

void EspWebSocket::onEvent(uint8_t c, WStype_t type, uint8_t *payload, size_t length) {
    if (type != WStype_TEXT)
        return;
    StaticJsonDocument<EVENT_MSG_LEN> doc;
    deserializeJson(doc, payload, length);

    if (!doc.containsKey(F("topic"))) {
        log_w("topic key missing %s", (char *) payload);
        return;
    }

    String topic = doc[F("topic")];
    HTTPMethod method = doc[F("method")] | HTTP_ANY;
    for (EventHandler *h = _firstHandler; h; h = h->next()) {
        if (h->canHandle(method, topic)) {
            h->handle(ws, c, topic);
            return;
        }
    }

    log_d("N/A | %d | %s", method, topic.c_str());
}

void EspWebSocket::addRequestHandler(EventHandler *handler) {
    if (!_lastHandler) {
        _firstHandler = handler;
        _lastHandler = handler;
    } else {
        _lastHandler->next(handler);
        _lastHandler = handler;
    }
}

EspWebSocket::EspWebSocket(uint16_t p, const char *secret) : ws(p) {
    ws.setAuthorization(secret);
}

void EspWebSocket::begin() {
    ws.onEvent(std::bind(&EspWebSocket::onEvent, this, std::placeholders::_1, std::placeholders::_2,
                         std::placeholders::_3, std::placeholders::_4));
    ws.begin();
}

EspWebSocket &
EspWebSocket::on(HTTPMethod m, const Uri &uri, EspRequestHandler fn, bool broadcast) {
    addRequestHandler(new EventHandler(std::move(fn), uri, m, broadcast));
    return *this;
}

void EspWebSocket::cycle() { ws.loop(); }

EspWebSocket &EspWebSocket::serve(Subscriber<EspWebSocket> &s) {
    s.subscribe(*this);
    return *this;
}

std::vector<String> EventHandler::topicArgs;

bool EventHandler::canHandle(HTTPMethod m, const String &to) {
    topicArgs.clear();
    return (method == HTTP_ANY || method == m) && topic->canHandle(to, topicArgs);
}

void EventHandler::handle(WebSocketsServer &ws, uint8_t num, const String &to) {
    EventRequest e(ws, num, to, method, broadcast, topicArgs);
    log_i("%d | %s", method, to.c_str());
    fn(&e);
}

EventHandler *EventHandler::next() { return _next; }

void EventHandler::next(EventHandler *e) { _next = e; }

String EventRequest::uri() const { return _topic; }

String EventRequest::pathArg(unsigned int i) const { return topicArgs[i]; }

void EventRequest::send(int code, const char *content_type, const String &content) {
    StaticJsonDocument<EVENT_MSG_LEN> doc;
    doc[F("topic")] = uri();
    doc[F("method")] = method;
    doc[F("code")] = code;

    if (content_type)
        doc[F("content-type")] = FPSTR(content_type);
    if (content && content.length())
        doc[F("message")] = content;

    String response;
    serializeJson(doc, response);

    if (broadcast)
        ws.broadcastTXT(response);
    else
        ws.sendTXT(client, response);
}
