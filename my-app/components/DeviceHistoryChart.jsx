"use client";

import React, { useEffect, useState, useRef } from "react";
import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
} from "recharts";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";

// ===================== CONFIGURAÇÃO =====================
const LASTN = 20;             // Últimos N pontos
const REFRESH_MS = 5000;      // Atualiza a cada 5 segundos
// ========================================================

export default function DeviceHistoryChart({ attr = "gb" }) {
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(false);
  const mounted = useRef(true);

  // Atualiza automaticamente
  useEffect(() => {
    mounted.current = true;
    fetchData();
    const timer = setInterval(() => fetchData(), REFRESH_MS);
    return () => {
      mounted.current = false;
      clearInterval(timer);
    };
  }, [attr]);

  async function fetchData() {
    setLoading(true);
    try {
      const url = `/api/sth?attr=${attr}&lastN=${LASTN}`;
      console.log(`📡 Buscando ${attr}:`, url);

      const resp = await fetch(url, {
        method: "GET",
        headers: {
          Accept: "application/json",
        },
      });

      if (!resp.ok) {
        console.error("❌ Erro STH:", resp.status);
        setLoading(false);
        return;
      }

      const json = await resp.json();
      let points = [];

      if (json.contextResponses && Array.isArray(json.contextResponses)) {
        const ce = json.contextResponses[0]?.contextElement;
        if (ce && Array.isArray(ce.attributes)) {
          const a = ce.attributes.find((x) => x.name === attr);
          if (a && Array.isArray(a.values)) {
            points = a.values.map((v) => ({
              ts: v.recvTime || "",
              value: Number(v.attrValue ?? v.value ?? 0),
            }));
          }
        }
      }

      const processed = points
        .map((p) => ({
          ...p,
          displayTs: formatShortTime(p.ts),
        }))
        .filter((p) => !Number.isNaN(p.value));

      if (mounted.current) setData(processed);
    } catch (err) {
      console.error("Erro ao buscar STH:", err);
    } finally {
      if (mounted.current) setLoading(false);
    }
  }

  function formatShortTime(iso) {
    if (!iso) return "";
    try {
      const d = new Date(iso);
      if (isNaN(d.getTime())) return iso;
      return d.toLocaleTimeString("pt-BR");
    } catch {
      return iso;
    }
  }

  // Define título com base no atributo
  const title = attr === "gb" ? "Gols - Time Azul" : "Gols - Time Vermelho";
  const color = attr === "gb" ? "#007bff" : "#dc3545";

  return (
    <Card className="shadow-lg border border-gray-200">
      <CardHeader>
        <CardTitle className="text-lg font-semibold text-center">
          {title}
        </CardTitle>
      </CardHeader>

      <CardContent>
        {loading && <div className="text-center text-sm text-gray-500">Carregando dados...</div>}

        {!loading && data.length === 0 && (
          <div className="text-center text-sm text-gray-500">
            Nenhum dado recebido ainda do STH-Comet.
          </div>
        )}

        {data.length > 0 && (
          <>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart
                data={data}
                margin={{ top: 20, right: 20, left: 10, bottom: 20 }}
              >
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="displayTs" />
                <YAxis allowDecimals={false} />
                <Tooltip
                  contentStyle={{ fontSize: 12 }}
                  formatter={(value) => [`${value}`, "Gols"]}
                  labelFormatter={(label) => `Horário: ${label}`}
                />
                <Bar dataKey="value" fill={color} barSize={25} />
              </BarChart>
            </ResponsiveContainer>

            <div className="mt-4">
              <h4 className="font-medium mb-2 text-sm">Últimos {data.length} eventos</h4>
              <table className="w-full text-sm border-collapse">
                <thead>
                  <tr className="border-b border-gray-200">
                    <th className="text-left pb-1">Horário</th>
                    <th className="text-left pb-1">Valor</th>
                  </tr>
                </thead>
                <tbody>
                  {data.map((p, i) => (
                    <tr key={i}>
                      <td className="py-1">{p.displayTs || p.ts}</td>
                      <td className="py-1">{p.value}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </>
        )}
      </CardContent>
    </Card>
  );
}
