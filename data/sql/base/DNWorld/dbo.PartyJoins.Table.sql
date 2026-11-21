USE [dnworld]
GO
/****** Object:  Table [dbo].[PartyJoins]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PartyJoins](
	[PartyID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PartyOwnerFlag] [bit] NOT NULL,
	[CharacterTypeCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_PartyJoins] PRIMARY KEY CLUSTERED 
(
	[PartyID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[PartyJoins] ADD  CONSTRAINT [DF_PartyJoins_CharacterTypeCode]  DEFAULT ((0)) FOR [CharacterTypeCode]
GO
ALTER TABLE [dbo].[PartyJoins]  WITH CHECK ADD  CONSTRAINT [FK_Parties_PartyJoins_PartyID] FOREIGN KEY([PartyID])
REFERENCES [dbo].[Parties] ([PartyID])
GO
ALTER TABLE [dbo].[PartyJoins] CHECK CONSTRAINT [FK_Parties_PartyJoins_PartyID]
GO
