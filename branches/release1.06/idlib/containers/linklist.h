/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __LINKLIST_H__
#define __LINKLIST_H__

/*
==============================================================================

idLinkList

Circular linked list template

==============================================================================
*/

template< class type >
class idLinkList {
public:
						idLinkList();
						~idLinkList();

	bool				IsListEmpty( void ) const;
	bool				InList( void ) const;
	int					Num( void ) const;
	void				Clear( void );

	void				InsertBefore( idLinkList &node );
	void				InsertAfter( idLinkList &node );
	void				AddToEnd( idLinkList &node );
	void				AddToFront( idLinkList &node );

	void				Remove( void );

	// Added by SophisticatedZombie for DarkMod
	// This method removes a node from the list and updates the head pointer of
	// every node in the list to prevent the list form becoming circular when the head
	// is removed.
	void				RemoveHeadsafe( void );

	type *				Next( void ) const;
	type *				Prev( void ) const;

	type *				Owner( void ) const;
	void				SetOwner( type *object );

	idLinkList *		ListHead( void ) const;
	idLinkList *		NextNode( void ) const;
	idLinkList *		PrevNode( void ) const;
	idLinkList *		NextNodeCircular( void ) const;
	idLinkList *		PrevNodeCircular( void ) const;

private:
	idLinkList *		head;
	idLinkList *		next;
	idLinkList *		prev;
	type *				owner;
};

/*
================
idLinkList<type>::idLinkList

Node is initialized to be the head of an empty list
================
*/
template< class type >
idLinkList<type>::idLinkList() {
	owner	= NULL;
	head	= this;	
	next	= this;
	prev	= this;
}

/*
================
idLinkList<type>::~idLinkList

Removes the node from the list, or if it's the head of a list, removes
all the nodes from the list.
================
*/
template< class type >
idLinkList<type>::~idLinkList() {
	Clear();
}

/*
================
idLinkList<type>::IsListEmpty

Returns true if the list is empty.
================
*/
template< class type >
bool idLinkList<type>::IsListEmpty( void ) const {
	return head->next == head;
}

/*
================
idLinkList<type>::InList

Returns true if the node is in a list.  If called on the head of a list, will always return false.
================
*/
template< class type >
bool idLinkList<type>::InList( void ) const {
	return head != this;
}

/*
================
idLinkList<type>::Num

Returns the number of nodes in the list.
================
*/
template< class type >
int idLinkList<type>::Num( void ) const {
	idLinkList<type>	*node;
	int					num;

	num = 0;
	for( node = head->next; node != head; node = node->next ) {
		num++;
	}

	return num;
}

/*
================
idLinkList<type>::Clear

If node is the head of the list, clears the list.  Otherwise it just removes the node from the list.
================
*/
template< class type >
void idLinkList<type>::Clear( void ) {
	if ( head == this ) {
		while( next != this ) {
			next->Remove();
		}
	} else {
		Remove();
	}
}

/*
================
idLinkList<type>::Remove

Removes node from list
================
*/
template< class type >
void idLinkList<type>::Remove( void ) {
	prev->next = next;
	next->prev = prev;

	next = this;
	prev = this;
	head = this;
}

/*
================
idLinkList<type>::RemoveHeadsafe

Removes node from list and updates
the list's head pointer if this was the head
================
*/
template< class type >
void idLinkList<type>::RemoveHeadsafe( void ) {

	// If this is the head, must walk list and tell all our little buddies
	// that we are no longer the head, and that the next node now is.
	if (head == this)
	{
		idLinkList<type>* p_cursor = next;

		while ((p_cursor != NULL) && (p_cursor != this))
		{
			p_cursor->head = next;
			p_cursor = p_cursor->next;
		}
	}

	// Now back to ID's original code
	prev->next = next;
	next->prev = prev;

	next = this;
	prev = this;
	head = this;


}

/*
================
idLinkList<type>::InsertBefore

Places the node before the existing node in the list.  If the existing node is the head,
then the new node is placed at the end of the list.
================
*/
template< class type >
void idLinkList<type>::InsertBefore( idLinkList &node ) {
	Remove();

	next		= &node;
	prev		= node.prev;
	node.prev	= this;
	prev->next	= this;
	head		= node.head;
}

/*
================
idLinkList<type>::InsertAfter

Places the node after the existing node in the list.  If the existing node is the head,
then the new node is placed at the beginning of the list.
================
*/
template< class type >
void idLinkList<type>::InsertAfter( idLinkList &node ) {
	Remove();

	prev		= &node;
	next		= node.next;
	node.next	= this;
	next->prev	= this;
	head		= node.head;
}

/*
================
idLinkList<type>::AddToEnd

Adds node at the end of the list
================
*/
template< class type >
void idLinkList<type>::AddToEnd( idLinkList &node ) {
	InsertBefore( *node.head );
}

/*
================
idLinkList<type>::AddToFront

Adds node at the beginning of the list
================
*/
template< class type >
void idLinkList<type>::AddToFront( idLinkList &node ) {
	InsertAfter( *node.head );
}

/*
================
idLinkList<type>::ListHead

Returns the head of the list.  If the node isn't in a list, it returns
a pointer to itself.
================
*/
template< class type >
idLinkList<type> *idLinkList<type>::ListHead( void ) const {
	return head;
}

/*
================
idLinkList<type>::Next

Returns the next object in the list, or NULL if at the end.
================
*/
template< class type >
type *idLinkList<type>::Next( void ) const {
	if ( !next || ( next == head ) ) {
		return NULL;
	}
	return next->owner;
}

/*
================
idLinkList<type>::Prev

Returns the previous object in the list, or NULL if at the beginning.
================
*/
template< class type >
type *idLinkList<type>::Prev( void ) const {
	if ( !prev || ( prev == head ) ) {
		return NULL;
	}
	return prev->owner;
}

/*
================
idLinkList<type>::NextNode

Returns the next node in the list, or NULL if at the end.
================
*/
template< class type >
idLinkList<type> *idLinkList<type>::NextNode( void ) const {
	if ( next == head ) {
		return NULL;
	}
	return next;
}

/*
================
idLinkList<type>::PrevNode

Returns the previous node in the list, or NULL if at the beginning.
================
*/
template< class type >
idLinkList<type> *idLinkList<type>::PrevNode( void ) const {
	if ( prev == head ) {
		return NULL;
	}
	return prev;
}

/*
================
idLinkList<type>::NextNodeCircular

Returns the next node in the list, possibly returning the head.
================
*/
template< class type >
idLinkList<type> *idLinkList<type>::NextNodeCircular( void ) const {
	return next;
}

/*
================
idLinkList<type>::PrevNodeCircular

Returns the previous node in the list, possibly returning the head.
================
*/
template< class type >
idLinkList<type> *idLinkList<type>::PrevNodeCircular( void ) const {
	return prev;
}

/*
================
idLinkList<type>::Owner

Gets the object that is associated with this node.
================
*/
template< class type >
type *idLinkList<type>::Owner( void ) const {
	return owner;
}

/*
================
idLinkList<type>::SetOwner

Sets the object that this node is associated with.
================
*/
template< class type >
void idLinkList<type>::SetOwner( type *object ) {
	owner = object;
}

#endif /* !__LINKLIST_H__ */
