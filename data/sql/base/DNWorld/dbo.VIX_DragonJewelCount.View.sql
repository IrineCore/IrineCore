USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_DragonJewelCount]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_DragonJewelCount] WITH SCHEMABINDING AS
SELECT DJ.ParentItemSerial, COUNT_BIG(*) DragonJewelCount
FROM dbo.DragonJewels DJ
GROUP BY DJ.ParentItemSerial
GO
