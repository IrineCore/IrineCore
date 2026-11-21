USE [dnmembership]
GO
/****** Object:  View [dbo].[VIX_CashIncomeTotals]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[VIX_CashIncomeTotals]'))
EXEC dbo.sp_executesql @statement = N'CREATE VIEW [dbo].[VIX_CashIncomeTotals]
WITH SCHEMABINDING
AS
SELECT AccountID, SUM(CAST(CashAmount AS bigint)) AS CashAmount, COUNT_BIG(*) AS CashIncomeCount
FROM dbo.CashIncome
GROUP BY AccountID;
' 
GO
SET ARITHABORT ON
SET CONCAT_NULL_YIELDS_NULL ON
SET QUOTED_IDENTIFIER ON
SET ANSI_NULLS ON
SET ANSI_PADDING ON
SET ANSI_WARNINGS ON
SET NUMERIC_ROUNDABORT OFF
GO
/****** Object:  Index [IX_UC_VIX_CashIncomeTotals]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[VIX_CashIncomeTotals]') AND name = N'IX_UC_VIX_CashIncomeTotals')
CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_CashIncomeTotals] ON [dbo].[VIX_CashIncomeTotals]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
