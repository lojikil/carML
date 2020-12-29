# The following fragments cause an error in carML/C currently:

-        TWHEN => (self-tco? name (-> src $ -> $ get children 1))
+        TWHEN => (self-tco? name (-> src $ get children 1))
         TIF => (any-of
-                (self-tco? name (-> src $ -> $ get children 1))
-                (self-tco? name (-> src $ -> $ get children 2)))
+                (self-tco? name (-> src $ get children 1))
+                (self-tco? name (-> src $ get children 2)))

# what I think is happening is that the `->` is a `coperator`, so
# it just assumes that there are two parameters, but one is actually
# nil. note what the `$` expands to:

(self-tco? name (-> src (-> (getchildren 1))))

# which then calls `llcwalk` or `llgwalk` on `head->children[1]`, which
# would be nil

# another test case:
(foo bar
# will cause a sigsegv
