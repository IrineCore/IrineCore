USE [dnstaging]
GO
/****** Object:  View [dbo].[V_Appellations]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_Appellations]
AS
SELECT DA.AccountID, DA.AccountName, DC.WorldID, DC.CharacterID, DC.CharacterName, DM.AppellationID, DM.AppellationName
FROM dbo.SS_Appellations SS WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON SS.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	INNER JOIN dbo.DimAppellations DM WITH (NOLOCK) ON SS.AppellationID = DM.AppellationID
WHERE DC.DeleteFlag = CAST(0 AS bit)
GO
