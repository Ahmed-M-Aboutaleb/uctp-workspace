// app/dashboard/loading.tsx
import { Card, CardContent, CardHeader } from "@/components/ui/card"

export default function loading() {
  return (
    <div className="space-y-8 p-6 animate-pulse">
      {/* Header Skeleton */}
      <div className="space-y-3">
        <div className="h-10 w-1/3 rounded-lg bg-muted" />
        <div className="h-4 w-1/2 rounded-lg bg-muted/60" />
      </div>

      {/* Stats Grid Skeleton */}
      <div className="grid grid-cols-2 gap-4 lg:grid-cols-4">
        {Array.from({ length: 4 }).map((_, i) => (
          <div key={i} className="h-32 rounded-2xl border border-border bg-card p-5">
            <div className="flex flex-col gap-4">
              <div className="h-12 w-12 rounded-2xl bg-muted" />
              <div className="space-y-2">
                <div className="h-3 w-16 rounded bg-muted" />
                <div className="h-6 w-12 rounded bg-muted" />
              </div>
            </div>
          </div>
        ))}
      </div>

      {/* Main Content Skeleton */}
      <div className="grid gap-6 lg:grid-cols-3">
        <div className="lg:col-span-2 h-[400px] rounded-3xl border border-border bg-card" />
        <div className="h-[400px] rounded-3xl border border-border bg-card" />
      </div>
    </div>
  )
}