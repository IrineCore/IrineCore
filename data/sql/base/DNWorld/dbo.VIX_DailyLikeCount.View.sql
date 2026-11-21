USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_DailyLikeCount]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_DailyLikeCount] 
WITH SCHEMABINDING AS
SELECT DL.CharacterID, COUNT_BIG(*) AS LikeCount
FROM dbo.DailyLikes DL
GROUP BY DL.CharacterID
GO
