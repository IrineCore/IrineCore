USE [dnworld]
GO
/****** Object:  Table [dbo].[Characters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Characters](
	[CharacterID] [bigint] NOT NULL,
	[AccountID] [int] NOT NULL,
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[AccountLevelCode] [tinyint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterIndex] [tinyint] NOT NULL,
	[VillageFirstVisitFlag] [bit] NOT NULL,
	[DefaultBody] [int] NOT NULL,
	[DefaultLeg] [int] NOT NULL,
	[DefaultHand] [int] NOT NULL,
	[DefaultFoot] [int] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
	[SkillResetFlag] [bit] NOT NULL,
	[DeleteDate] [datetime2](0) NULL,
	[WorldID] [int] NOT NULL,
	[DeleteByAdminFlag] [bit] NULL,
	[CreateDate] [datetime2](0) NOT NULL,
	[MobileAuthenticationFlag] [bit] NULL,
	[PrivateChannelID] [bigint] NULL,
 CONSTRAINT [PK_Characters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_Characters_AccountName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Characters_AccountName] ON [dbo].[Characters]
(
	[AccountName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_Characters_PrivateChannelID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Characters_PrivateChannelID] ON [dbo].[Characters]
(
	[PrivateChannelID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Characters_AccountID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Characters_AccountID] ON [dbo].[Characters]
(
	[AccountID] ASC
)
INCLUDE([WorldID],[DeleteFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UNF_Characters_CharacterName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_Characters_CharacterName] ON [dbo].[Characters]
(
	[CharacterName] ASC
)
WHERE ([DeleteFlag]=CONVERT([bit],(0),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[Characters] ADD  CONSTRAINT [DF_Characters_CreateDate]  DEFAULT (getdate()) FOR [CreateDate]
GO
