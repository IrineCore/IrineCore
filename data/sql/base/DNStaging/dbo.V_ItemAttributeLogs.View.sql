USE [dnstaging]
GO
/****** Object:  View [dbo].[V_ItemAttributeLogs]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE VIEW [dbo].[V_ItemAttributeLogs]
	AS
	SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130913 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130914 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130915 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130916 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130917 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130918 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130919 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130920 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20130921 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20131003 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20131004 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20131005 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20131006 UNION ALL
SELECT * FROM DNStaging.dbo.ItemAttributeLogs_20131007
GO
