namespace octet 
{
	template<class T, class allocator_t = allocator>
	class circular_list 
	{
		struct item{
			void *operator new(size_t size) {
				return allocator_t::malloc(size);
			}
			void operator delete(void *ptr, size_t size) {
				return allocator_t::free(ptr, size);
			}
			item(const T &value) : value(value)
			{
			}
			T value; 
			item *prev; 
			item *next;
		};
		item *head;
		int count;
	public:
		circular_list() : head(NULL),
		count(0)
		{
		}

		void clear()
		{
			count = 0;
			if(head == NULL)
			{
				return;
			}
			item *p = head;
			while(p->next != head)
			{
				p = p->next;
				delete p->prev;
			}
			delete p;
			head = NULL;
		}

		int size()
		{
			return count;
		}

		void push_back(const T &value) {
			item *p = new item(value);
			count++;
			if(head == NULL)
			{
				p->prev = p->next = p;
				head = p;
				return;
			}
			item *tail = head;
			while(tail->next != head)
			{
				tail = tail->next;
			}
			p->prev = tail;
			p->next = head;
			head->prev = p;
			tail->next = p;
		}

		class iterator
		{
			item *node;
			friend class circular_list;
		public:
			iterator(item *i) : node(i)
			{
			}

			iterator &operator--()
			{
				node = node->prev;
				return *this;
			}

			iterator &operator++()
			{
				node = node->next;
				return *this;
			}

			T *operator->()
			{
				return &node->value;
			}

			T &operator*()
			{
				return node->value;
			}
		};

		bool insert_after(iterator &iter, const T &value) 
		{
			if(head == NULL)
				return false;
			item *prev = iter.node;
			item *p = head;
			while(prev != p && p->next != head)
			{
				p = p->next;
			}
			if(p != prev)
			{
				return false;
			}
			item *p1 = new item(value);
			p1->next = p->next;
			p1->prev = p;
			if(p->next != NULL)
			{
				p->next->prev = p1;
			}
			p->next = p1;
			count++;
			return true;
		}

		iterator begin()
		{
			return iterator(head);
		}

		bool remove(iterator &iter)
		{
			if(head == NULL)
				return false;
			item *victim = iter.node;
			item *p = head;
			while(p != victim && p->next != head)
			{
				p = p->next;
			}
			if(p == victim)
			{
				victim->prev->next = victim->next;
				victim->next->prev = victim->prev;
				if(victim == head)
				{
					if(head == head->next)
						head = NULL;
					else
						head = head->next;
				}
				delete victim;
				count--;
			}
			return false;
		}
	};
}
