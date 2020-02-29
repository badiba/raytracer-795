#ifndef _BTNODE_H_
#define _BTNODE_H_

template <class T>
class BTNode
{
public:
	T data;
	BTNode<T> *left;
	BTNode<T> *right;

	BTNode();
	BTNode(T data, BTNode<T> *left, BTNode<T> *right);
};

template <class T>
BTNode<T>::BTNode() {

	this->left = nullptr;
	this->right = nullptr;
}

template <class T>
BTNode<T>::BTNode(T data, BTNode<T> *left, BTNode<T> *right) {

	this->data = data;
	this->left = left;
	this->right = right;
}

#endif
