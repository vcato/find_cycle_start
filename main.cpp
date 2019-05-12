#include <cassert>
#include <vector>
#include <limits>

using std::vector;


namespace {
class Path {
  public:
    using NodeIndex = size_t;

    static NodeIndex end()
    {
      return std::numeric_limits<NodeIndex>::max();
    }

    NodeIndex addNode()
    {
      NodeIndex index = _nodes.size();
      _nodes.emplace_back();
      return index;
    }

    void setNext(NodeIndex node_index,NodeIndex next_node_index)
    {
      if (node_index == end()) {
        checkNodeIndex(next_node_index);
        _start = next_node_index;
        return;
      }

      checkMaybeNodeIndex(next_node_index);
      node(node_index).next = next_node_index;
    }

    NodeIndex next(NodeIndex i) const
    {
      return node(i).next;
    }

    NodeIndex start() const
    {
      return _start;
    }

  private:
    struct Node {
      NodeIndex next = end();
    };

    NodeIndex _start = end();
    vector<Node> _nodes;

    const Node &node(NodeIndex i) const
    {
      checkNodeIndex(i);
      return _nodes[i];
    }

    Node &node(NodeIndex i)
    {
      checkNodeIndex(i);
      return _nodes[i];
    }

    void checkMaybeNodeIndex(NodeIndex node_index) const
    {
      if (node_index == end()) return;

      checkNodeIndex(node_index);
    }

    void checkNodeIndex(NodeIndex i) const
    {
      assert(i < _nodes.size());
    }
};
}


using NodeIndex = Path::NodeIndex;


static NodeIndex nextNodeIndex(const Path &p,NodeIndex n,size_t count)
{
  while (count > 0) {
    n = p.next(n);
    --count;
  }

  return n;
}


static bool isEmpty(const Path &p)
{
  return p.start() == p.end();
}


static size_t nNodesInCycle(const Path &p,NodeIndex cycle_start)
{
  NodeIndex i = cycle_start;

  if (cycle_start == p.end()) {
    return 0;
  }

  size_t n_nodes_in_cycle = 0;

  for (;;) {
    assert(i != p.end());
    i = p.next(i);
    ++n_nodes_in_cycle;

    if (i == cycle_start) {
      break;
    }
  }

  return n_nodes_in_cycle;
}


static NodeIndex startOfCycleContaining(const Path &path,NodeIndex i)
{
  size_t cycle_size = nNodesInCycle(path,i);
  NodeIndex i1 = path.start();
  NodeIndex i2 = nextNodeIndex(path,i1,cycle_size);

  while (i1 != i2) {
    i1 = path.next(i1);
    i2 = path.next(i2);
  }

  return i1;
}


static NodeIndex findCycle(const Path &path)
{
  NodeIndex start = path.start();

  if (start == path.end()) {
    return start;
  }

  NodeIndex i1 = start;
  NodeIndex i2 = start;

  for (;;) {
    i2 = path.next(i2);
    
    if (i2 == path.end()) {
      return i2;
    }

    if (i2 == i1) {
      return startOfCycleContaining(path,i1);
    }

    i2 = path.next(i2);

    if (i2 == path.end()) {
      return i2;
    }

    if (i2 == i1) {
      return startOfCycleContaining(path,i1);
    }

    i1 = path.next(i1);

    if (i1 == i2) {
      // If i2 did not run into i1, but i1 ran into i2, then it has to
      // be at the start of the cycle.
      return i1;
    }
  }
}


static Path createPath(size_t n_nodes_before_cycle,size_t n_nodes_in_cycle)
{
  Path p;
  NodeIndex prev_node_index = p.end();

  for (size_t i=0; i!=n_nodes_before_cycle; ++i) {
    NodeIndex node_index = p.addNode();
    p.setNext(prev_node_index,node_index);
    prev_node_index = node_index;
  }

  NodeIndex cycle_start = p.end();

  for (size_t i=0; i!=n_nodes_in_cycle; ++i) {
    NodeIndex node_index = p.addNode();

    if (cycle_start == p.end()) {
      cycle_start = node_index;
    }

    p.setNext(prev_node_index,node_index);
    prev_node_index = node_index;
  }

  if (prev_node_index != p.end()) {
    p.setNext(prev_node_index,cycle_start);
  }

  return p;
}


static bool hasCycle(const Path &p)
{
  return findCycle(p) != p.end();
}


static size_t nNodesBefore(const Path &p,NodeIndex node_index)
{
  size_t n = 0;
  NodeIndex i = p.start();

  while (i != node_index) {
    i = p.next(i);
    ++n;
  }

  return n;
}


static void testCreatePath()
{
  {
    Path p = createPath(/*n_nodes_before_cycle*/0,/*n_nodes_in_cycle*/0);
    assert(isEmpty(p));
  }
  {
    Path p = createPath(/*n_nodes_before_cycle*/1,/*n_nodes_in_cycle*/0);
    assert(!isEmpty(p));
    assert(p.next(p.start()) == p.end());
  }
  {
    Path p = createPath(/*n_nodes_before_cycle*/0,/*n_nodes_in_cycle*/1);
    assert(p.next(p.start()) == p.start());
  }
  {
    Path p = createPath(/*n_nodes_before_cycle*/1,/*n_nodes_in_cycle*/1);
    NodeIndex n1 = p.start();
    NodeIndex n2 = p.next(n1);
    assert(n2 != p.end());
    assert(p.next(n2) == n2);
  }
  {
    Path p = createPath(/*n_nodes_before_cycle*/2,/*n_nodes_in_cycle*/1);
    assert(!isEmpty(p));
    NodeIndex n1 = p.start();
    NodeIndex n2 = p.next(n1);
    NodeIndex n3 = p.next(n2);
    assert(p.next(n3) == n3);
  }
  {
    Path p = createPath(/*n_nodes_before_cycle*/0,/*n_nodes_in_cycle*/2);
    assert(!isEmpty(p));
    NodeIndex n1 = p.start();
    NodeIndex n2 = p.next(n1);
    assert(n1!=n2);
    assert(p.next(n2) == n1);
  }
}


static void testWithEmptyPath()
{
  Path p = createPath(/*n_nodes_before_cycle*/0,/*n_nodes_in_cycle*/0);
  assert(!hasCycle(p));
  assert(nNodesInCycle(p,findCycle(p)) == 0);
}


static void testWithOneNodeWithoutCycle()
{
  Path p = createPath(/*n_nodes_before_cycle*/1,/*n_nodes_in_cycle*/0);
  assert(!hasCycle(p));
}


static void testWithOneNodeWithCycle()
{
  Path p = createPath(/*n_nodes_before_cycle*/0,/*n_nodes_in_cycle*/1);
  NodeIndex cycle_start = findCycle(p);
  assert(cycle_start != p.end());
  assert(cycle_start == p.start());
  assert(nNodesInCycle(p,cycle_start) == 1);
}


static void testSimplePathWithoutCycle()
{
  Path p = createPath(/*n_nodes_before_cycle*/2,/*n_nodes_in_cycle*/0);
  assert(!hasCycle(p));
}


static void testSimplePathWithCycle()
{
  Path p = createPath(/*n_nodes_before_cycle*/1,/*n_nodes_in_cycle*/1);
  NodeIndex cycle_start = findCycle(p);
  assert(!isEmpty(p));
  NodeIndex n1 = p.start();
  assert(p.next(n1) != p.end());
  NodeIndex n2 = p.next(n1);
  assert(cycle_start == n2);
  assert(nNodesInCycle(p,cycle_start)==1);
}


static void testPathsWithoutCycle()
{
  for (size_t n=0; n!=10; ++n) {
    Path p = createPath(/*n_nodes_before_cycle**/n,/*n_nodes_in_cycle*/0);
    assert(!hasCycle(p));
  }
}


static void testPathsWithCycle()
{
  for (size_t n=0; n!=10; ++n) {
    for (size_t c=0; c!=10; ++c) {
      Path p = createPath(/*n_nodes_before_cycle*/n,/*n_nodes_in_cycle*/c);
      NodeIndex cycle_start = findCycle(p);
      size_t n_nodes_before_cycle = nNodesBefore(p,cycle_start);
      size_t n_nodes_in_cycle = nNodesInCycle(p,cycle_start);
      assert(n_nodes_before_cycle == n);
      assert(n_nodes_in_cycle == c);
    }
  }
}


int main()
{
  testCreatePath();
  testWithEmptyPath();
  testWithOneNodeWithoutCycle();
  testWithOneNodeWithCycle();
  testSimplePathWithoutCycle();
  testSimplePathWithCycle();
  testPathsWithoutCycle();
  testPathsWithCycle();
}
