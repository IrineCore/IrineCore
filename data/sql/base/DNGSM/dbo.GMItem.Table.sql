USE [dngsm]
GO
/****** Object:  Table [dbo].[GMItem]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMItem](
	[ItemID] [int] NOT NULL,
	[ItemStringID] [int] NOT NULL,
	[Dur] [smallint] NOT NULL,
	[RandomSeed] [int] NOT NULL,
	[CoolTime] [int] NOT NULL,
	[Color] [int] NOT NULL,
	[Level] [tinyint] NOT NULL,
	[Potential] [tinyint] NOT NULL,
	[Soulbound] [bit] NOT NULL,
	[Identify] [bit] NOT NULL,
 CONSTRAINT [PK_GMItem] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Dur]  DEFAULT ((0)) FOR [Dur]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_RandomSeed]  DEFAULT ((0)) FOR [RandomSeed]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_CoolTime]  DEFAULT ((0)) FOR [CoolTime]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Color]  DEFAULT ((0)) FOR [Color]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Level]  DEFAULT ((0)) FOR [Level]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Potential]  DEFAULT ((0)) FOR [Potential]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Soulbound]  DEFAULT ((0)) FOR [Soulbound]
GO
ALTER TABLE [dbo].[GMItem] ADD  CONSTRAINT [DF_GMItem_Identify]  DEFAULT ((0)) FOR [Identify]
GO
