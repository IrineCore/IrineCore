USE [dnmembership]
GO
/****** Object:  View [dbo].[VIX_EventScheduleDetailForPetal]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[VIX_EventScheduleDetailForPetal]'))
EXEC dbo.sp_executesql @statement = N'CREATE VIEW [dbo].[VIX_EventScheduleDetailForPetal]
WITH SCHEMABINDING
AS
SELECT ESD.AccountID, ES.EventConditionID, COUNT_BIG(*) AS Cnt
FROM dbo.EventScheduleForPetal ES
	INNER JOIN dbo.EventScheduleDetailForPetal ESD ON ES.EventScheduleID = ESD.EventScheduleID
WHERE ES.EventConditionID IS NOT NULL
GROUP BY ESD.AccountID, ES.EventConditionID
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
/****** Object:  Index [IX_UC_VIX_EventScheduleDetailForPetal]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[VIX_EventScheduleDetailForPetal]') AND name = N'IX_UC_VIX_EventScheduleDetailForPetal')
CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_EventScheduleDetailForPetal] ON [dbo].[VIX_EventScheduleDetailForPetal]
(
	[AccountID] ASC,
	[EventConditionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
