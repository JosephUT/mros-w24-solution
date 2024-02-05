# Mediator and Node Interaction

## Mediator

### addNode()
- connecting callback

### addPublisher()
- called by create publisher

### addSubscriber()
- called by create subscriber

### removeNode()
- registered as closing callback inside lambda to feed in specific connection node uri
- called on all nodes during mediator shutdown
- potentially called by another node that mimics `rosnode kill` command line tool

## Node

### connectToPublishers()