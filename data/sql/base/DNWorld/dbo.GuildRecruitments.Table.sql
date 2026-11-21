USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildRecruitments]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildRecruitments](
	[GuildID] [int] NOT NULL,
	[GuildIntroduction] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PurposeCode] [tinyint] NOT NULL,
	[MinLevel] [tinyint] NOT NULL,
	[MaxLevel] [tinyint] NOT NULL,
	[GuildHomepageURLFlag] [bit] NULL,
	[CharacterClassCodeBitmap] [int] NULL,
	[GuildName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[GuildLevel] [tinyint] NULL,
 CONSTRAINT [PK_GuildRecruitments] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
ALTER TABLE [dbo].[GuildRecruitments] ADD  CONSTRAINT [DF_GuildRecruitments_PurposeCode]  DEFAULT ((1)) FOR [PurposeCode]
GO
ALTER TABLE [dbo].[GuildRecruitments] ADD  CONSTRAINT [DF_GuildRecruitments_GuildHomepageURLFlag]  DEFAULT ((0)) FOR [GuildHomepageURLFlag]
GO
