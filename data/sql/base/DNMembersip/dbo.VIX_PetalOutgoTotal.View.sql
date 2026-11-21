USE [dnmembership]
GO
/****** Object:  View [dbo].[VIX_PetalOutgoTotal]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[VIX_PetalOutgoTotal]'))
EXEC dbo.sp_executesql @statement = N'CREATE VIEW [dbo].[VIX_PetalOutgoTotal]
WITH SCHEMABINDING
AS
SELECT PetalIncomeID, SUM(PetalAmount) AS PetalAmount, COUNT_BIG(*) AS Cnt
FROM dbo.PetalOutgo
GROUP BY PetalIncomeID
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
/****** Object:  Index [IX_UN_VIX_PetalOutgoTotal]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[VIX_PetalOutgoTotal]') AND name = N'IX_UN_VIX_PetalOutgoTotal')
CREATE UNIQUE CLUSTERED INDEX [IX_UN_VIX_PetalOutgoTotal] ON [dbo].[VIX_PetalOutgoTotal]
(
	[PetalIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
