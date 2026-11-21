USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimCharacters]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimCharacters](
	[CharacterID] [bigint] NOT NULL,
	[AccountID] [int] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
	[WorldID] [int] NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterIndex] [tinyint] NOT NULL,
	[VillageFirstVisitFlag] [bit] NOT NULL,
	[SkillResetFlag] [bit] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterExp] [int] NOT NULL,
	[MissionScore] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[Coin] [bigint] NOT NULL,
	[WarehouseCoin] [bigint] NOT NULL,
	[SkillPoint] [smallint] NOT NULL,
	[LastLoginDate] [datetime2](0) NOT NULL,
	[CreateDate] [datetime2](0) NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[DeleteDate] [datetime2](0) NULL,
	[DeleteCharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
	[DeletePrivateIP] [int] NULL,
	[DeletePublicIP] [int] NULL,
	[AccountLevelCode] [tinyint] NULL,
	[PCRoomRebirthCoin] [tinyint] NULL,
	[RebirthCoin] [tinyint] NULL,
 CONSTRAINT [PK_DimCharacters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_DimCharacters_CharacterName]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DimCharacters_CharacterName] ON [dbo].[DimCharacters]
(
	[CharacterName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_DimCharacters_DeleteCharacterName]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DimCharacters_DeleteCharacterName] ON [dbo].[DimCharacters]
(
	[DeleteCharacterName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NNI_DimCharacters_AccountID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_DimCharacters_AccountID] ON [dbo].[DimCharacters]
(
	[AccountID] ASC
)
INCLUDE([CharacterID],[CharacterName],[WorldID]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
