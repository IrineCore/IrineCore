USE [dnstaging]
GO
/****** Object:  View [dbo].[V_CharactersForNexon]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[V_CharactersForNexon]
AS
SELECT MC.AccountName, MC.CharacterID, MC.CharacterName, MC.CharacterLevel, MC.JobCode, MC.GuildName, MC.WorldID, W.WorldName, NULL AS CharacterIDForNexon
FROM dbo.MirroredCharacters MC WITH (NOLOCK)
	INNER JOIN dbo.DimWorlds W WITH (NOLOCK) ON MC.WorldID = W.WorldID;
GO
