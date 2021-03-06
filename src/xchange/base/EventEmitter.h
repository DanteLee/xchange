#ifndef XCHANGE_BASE_EVENTEMITTER_H_
#define XCHANGE_BASE_EVENTEMITTER_H_

#include <map>
#include <vector>
#include <functional>

namespace xchange {

    template<typename T>
    class EventEmitter {
        public:
            typedef std::function<void (T, void*)> EventHandler;
            typedef std::vector<EventHandler> HandlerList;

            EventEmitter():hasEventHandler_(false) {};
            ~EventEmitter() {};

            void on(T e, EventHandler callback) {
                typename std::map<T, HandlerList *>::iterator iter = events_.find(e);

                if (iter == events_.end()) {
                    HandlerList *list = new HandlerList();

                    if (callback != NULL) {
                        hasEventHandler_ = true;
                        list->push_back(callback);
                    }

                    events_.insert(typename std::map<T, HandlerList *>::value_type(e, list));
                } else {
                    HandlerList *list = iter->second;

                    if (callback != NULL) {
                        hasEventHandler_ = true;
                        list->push_back(callback);
                    }
                }
            };

            void emit(T e, void * arg = NULL) {
                typename std::map<T, HandlerList *>::iterator iter = events_.find(e);

                if (iter != events_.end()) {
                    HandlerList & list = *(iter->second);

                    for (typename HandlerList::iterator i = list.begin(); i != list.end(); i++) {
                        (*i)(e, arg);
                    }
                }
            };

            bool hasEventHandler() const {return hasEventHandler_;}
        protected:
            bool hasEventHandler_;
            std::map<T, HandlerList *> events_;
    };
}

#endif
