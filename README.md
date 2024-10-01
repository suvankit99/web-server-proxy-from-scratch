Multi threaded proxy web server with LRU cache built from scratch

What is a proxy web server ? 

 proxy web server is an intermediary server that sits between a client (such as a web browser) and the internet. 
 When a client makes a request to access a resource (like a web page), the proxy server processes the request on 
 behalf of the client and forwards it to the target web server. Once the target server responds, 
 the proxy server sends the response back to the client.

 Use of mutex and semaphores : 
 Every client request requires us to create a new thread , and since these threads have the LRU cache as a shared resource , 
 it can cause race conditions , so to avoid it , we use mutex and semaphores 

 
