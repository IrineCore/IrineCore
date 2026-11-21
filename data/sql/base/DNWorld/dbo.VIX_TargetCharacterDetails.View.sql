USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_TargetCharacterDetails]
WITH SCHEMABINDING
AS
SELECT CompleteFlag, COUNT_BIG(*) AS Cnt
FROM dbo.TargetCharacterDetails
GROUP BY CompleteFlag
GO
CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_TargetCharacterDetails] ON [dbo].[VIX_TargetCharacterDetails]
(
	[CompleteFlag] ASC
) ON [IndexFG1]
GO
