USE [dnworld]
GO
/****** Object:  Table [dbo].[Profiles]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Profiles](
	[CharacterID] [bigint] NOT NULL,
	[GenderCode] [tinyint] NOT NULL,
	[SelfIntroduction] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PublicFlag] [bit] NOT NULL,
 CONSTRAINT [PK_Profiles] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
