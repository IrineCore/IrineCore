USE [dnstaging]
GO
/****** Object:  Table [dbo].[TempDeletedCharacters]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TempDeletedCharacters](
	[CharacterID] [bigint] NOT NULL,
	[DeleteDate] [datetime2](0) NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[DeletePrivateIP] [int] NULL,
	[DeletePublicIP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UC_TempDeletedCharacters_CharacterID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_TempDeletedCharacters_CharacterID] ON [dbo].[TempDeletedCharacters]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
