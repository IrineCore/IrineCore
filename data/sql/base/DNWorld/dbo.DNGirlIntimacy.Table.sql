USE [dnworld]
GO
/****** Object:  Table [dbo].[DNGirlIntimacy]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DNGirlIntimacy](
	[CharacterID] [bigint] NOT NULL,
	[DNGirlCharacterID] [bigint] NOT NULL,
	[Intimacy] [int] NOT NULL,
	[FanFlag] [bit] NOT NULL,
	[UpdateDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DNGirlIntimacy] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[DNGirlCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[DNGirlIntimacy] ADD  CONSTRAINT [DF_DNGirlIntimacy_Intimacy]  DEFAULT ((0)) FOR [Intimacy]
GO
ALTER TABLE [dbo].[DNGirlIntimacy] ADD  CONSTRAINT [DF_DNGirlIntimacy_UpdateDate]  DEFAULT (getdate()) FOR [UpdateDate]
GO
