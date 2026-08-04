export default function generatePaths(count) {
  const roots = [
    "/home/alice/projects",
    "/Users/alice/Developer",
    "/opt/services",
    "/var/www",
    "/mnt/data/repos",
  ];

  const dirs = [
    "src", "lib", "app", "packages", "apps", "modules",
    "components", "hooks", "utils", "features", "pages",
    "api", "controllers", "models", "services", "stores",
    "config", "scripts", "docs", "tests", "__tests__",
    "public", "assets", "images", "icons", "styles"
  ];

  const features = [
    "auth", "user", "search", "billing", "checkout", "orders",
    "products", "notifications", "dashboard", "profile",
    "analytics", "reports", "settings", "payments",
    "inventory", "chat", "admin", "marketing"
  ];

  const names = [
    "index", "main", "app", "client", "server",
    "Button", "Modal", "Table", "Input", "Card",
    "useAuth", "useFetch", "useSearch",
    "UserService", "AuthService", "ApiClient",
    "UserController", "OrderController",
    "UserModel", "ProductModel",
    "constants", "types", "helpers", "utils",
    "README", "package", "tsconfig", "vite.config"
  ];

  const exts = [
    "ts", "tsx", "js", "jsx", "json", "css",
    "scss", "md", "yml", "svg", "png"
  ];

  const paths = new Array(count);

  for (let i = 0; i < count; i++) {
    const depth = 2 + (Math.random() * 4 | 0); // 2-5 folders

    const parts = [
      roots[Math.random() * roots.length | 0]
    ];

    for (let d = 0; d < depth; d++) {
      const pool = d % 2 ? features : dirs;
      parts.push(pool[Math.random() * pool.length | 0]);
    }

    const name = names[Math.random() * names.length | 0];
    const ext = exts[Math.random() * exts.length | 0];

    const suffix = Math.random() < 0.2 ? "" : `-${Math.random() * 50 | 0}`;

    paths[i] = `${parts.join("/")}/${name}${suffix}.${ext}`;
  }

  return paths;
}
