# Courseware

This is an application for managing students and courses, allowing students to register, de-register and enroll for courses. Courses can also be created or deleted. Courseware maintains the current status of
students (registered, de-registered), courses (active, deleted) as well as enrollments. Enrollment can contain only registered students and active courses, subject to the capacity of the course.

![](courseware\algo.PNG)



Under weak isolation, it is possible that two different students, when trying to enroll concurrently, will both succeed even though only one spot was left in the course. Another example that breaks the application is when a student is trying to register for a course that is being concurrently removed: once the course is removed, no student should be seen as enrolled in that course.



# Shopping Cart

This application allows a user to add, remove and change quantity of items from different sessions. It also allows the user to view all items present in the shopping cart.



![](shopping_cart\algo.PNG)



Suppose that initially, a user $u$ has a single instance of item *i* in their cart. Then the user connects to the application via two different sessions (for instance, via two browser windows), adds *i* in one session (*AddItem(i, u)*) and deletes *i* in the other session (*DeleteItem(i, u)*).  With serializability, the cart can either be left in the state \{ *i* \} (delete happened first, followed by the add) or \{ \} (delete happened second). However, with causal consistency (or read committed), it is possible that with two sequential reads of the shopping cart, the cart is empty in the first read (signaling that the delete has succeeded), but there are *two* instances of *i* in the second read.



# Stack

Treiber stack is a concurrent stack data structure that uses compare-and-swap (CAS) instructions instead of locks for synchronization. This algorithm was ported to operate on a kv-store in prior work [[1]](https://arxiv.org/abs/2004.10158) and we use that implementation. Essentially, the stack contents are placed in a kv-store, instead of using an in-memory linked data structure.

![](treiber_stack\algo.PNG)

Each row in the store contains a pair consisting of the stack element and the key of the next
row down in the stack. A designated key *head* stores the key of the top of the stack. CAS is implemented as a transaction, but the *pop* and *push* operations do not use transactions, i.e., each read/write/CAS is its own transaction.



When two different clients try to *pop* from the stack concurrently, under serializability, each *pop* would return a unique value, assuming that each pushed value is unique. However, under causal consistency, concurrent *pops* can return the same value.



# Twitter

This is based on a social-networking application that allows users to create a new account, follow, unfollow, tweet, browse the newsfeed (tweets from users you follow) and the timeline of any particular user. 

![](twitter\algo.PNG)



A user can access twitter from multiple clients (sessions), which could lead to unexpected behavior under weak isolation levels. 
Consider the following scenario with two users, *A* and *B* where user *A* is accessing twitter from two different sessions, *S_1* and *S_2*. User *A* views the timeline of user *B* from one session (*S_1:Timeline(B)*) and decides to follow *B* through another session (*S_2: Follow(A, B)}*). Now when user *A* visits their timeline or newsfeed (*S_2: NewsFeed(A)*), they expect to see all the tweets of *B* that were visible via *Timeline* in session *S_1*. But under weak isolation levels, this does not always hold true and there could be missing tweets. 