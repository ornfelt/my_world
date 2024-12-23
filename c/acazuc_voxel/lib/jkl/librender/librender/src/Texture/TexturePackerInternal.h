#ifndef TEXTURE_PACKER_INTERNAL_H
# define TEXTURE_PACKER_INTERNAL_H

namespace librender
{

	struct WHRect
	{
		int32_t width;
		int32_t height;
		WHRect(int32_t width, int32_t height) : width(width), height(height) {};
	};

	struct XYWHRect
	{
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
		XYWHRect(int32_t x, int32_t y, int32_t width, int32_t height) : x(x), y(y), width(width), height(height) {};
		XYWHRect() {};
	};

	class Node
	{

		struct PNode
		{
			Node *node;
			bool fill;
			PNode() : fill(false), node(nullptr) {}
			~PNode() {delete (this->node);};
			void set(XYWHRect rect)
			{
				if (!this->node)
					this->node = new Node();
				this->node->rect = rect;
				this->fill = true;
			}
			void clear()
			{
				if (!this->node)
					return;
				this->fill = false;
				this->node->clear();
			}
		};

		public:
		PNode childs[2];
		XYWHRect rect;
		bool full;
		Node() : full(false) {};
		Node(XYWHRect rect) : full(false), rect(rect) {};
		void reset(WHRect rect)
		{
			this->rect = XYWHRect(0, 0, rect.width, rect.height);
			clear();
		}
		Node *insert(WHRect img)
		{
			if (this->childs[0].node && this->childs[0].fill)
			{
				if (Node *node = this->childs[0].node->insert(img))
					return node;
				return this->childs[1].node->insert(img);
			}
			if (this->full)
				return 0;
			if (img.width == this->rect.width && img.height == this->rect.height)
			{
				this->full = true;
				return this;
			}
			if (img.width > this->rect.width || img.height > this->rect.height)
				return nullptr;
			if (this->rect.width - img.width > this->rect.height - img.height)
			{
				this->childs[0].set(XYWHRect(this->rect.x, this->rect.y, img.width, this->rect.height));
				this->childs[1].set(XYWHRect(this->rect.x + img.width, this->rect.y, this->rect.width - img.width, this->rect.height));
			}
			else
			{
				this->childs[0].set(XYWHRect(this->rect.x, this->rect.y, this->rect.width, img.height));
				this->childs[1].set(XYWHRect(this->rect.x, this->rect.y + img.height, this->rect.width, this->rect.height - img.height));
			}
			return this->childs[0].node->insert(img);
		}
		void clear()
		{
			this->full = false;
			this->childs[0].clear();
			this->childs[1].clear();
		}

	};

}

#endif
